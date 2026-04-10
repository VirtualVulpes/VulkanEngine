#include "frame_resources.h"

#include "image.h"
#include "object.h"
#include "vulkan_device.h"
#include "texture.h"

FrameResources::FrameResources(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                               vk::DescriptorSetLayout descriptorSetLayout) {
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = device.getCommandPool(),
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    commandBuffer_ = std::move(vk::raii::CommandBuffers(device.getDevice(), allocateInfo).front());

    presentCompleteSemaphore_ = vk::raii::Semaphore(device.getDevice(), vk::SemaphoreCreateInfo());
    inFlightFence_ = vk::raii::Fence(device.getDevice(), {.flags = vk::FenceCreateFlagBits::eSignaled});

    uniformBuffer_ = Buffer::createAllocation(sizeof(FrameData),
                                             vk::BufferUsageFlagBits::eUniformBuffer,
                                             vk::MemoryPropertyFlagBits::eHostVisible |
                                             vk::MemoryPropertyFlagBits::eHostCoherent,
                                             device);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };
    descriptorSet_ = std::move(device.getDevice().allocateDescriptorSets(allocInfo).front());


    vk::DescriptorBufferInfo bufferInfo{
        .buffer = *uniformBuffer_.buffer,
        .offset = 0,
        .range = sizeof(FrameData)
    };

    vk::WriteDescriptorSet write {
        .dstSet = descriptorSet_,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo
    };

    device.getDevice().updateDescriptorSets(write, {});
}

void FrameResources::beginFrame(const vk::raii::Device& device) {
    auto fenceResult = device.waitForFences(*inFlightFence_, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fence");
    }
    commandBuffer_.reset();
}

void FrameResources::recordCommandBuffer(vk::Pipeline graphicsPipeline,
                                         vk::PipelineLayout pipelineLayout,
                                         vk::DescriptorSet frameDescriptorSet, RenderTarget renderTarget,
                                         uint32_t frameIndex,
                                         std::span<const Object* const> objects) const {
    commandBuffer_.begin({});

    Image::transitionLayout(
        renderTarget.swapChainImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer_
    );
    Image::transitionLayout(
        renderTarget.colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer_
    );
    Image::transitionLayout(
        renderTarget.depthImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthAttachmentOptimal,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
        vk::ImageAspectFlagBits::eDepth,
        commandBuffer_
    );

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderingAttachmentInfo colorAttachmentInfo = {
        .imageView = renderTarget.colorImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .resolveMode = vk::ResolveModeFlagBits::eAverage,
        .resolveImageView = renderTarget.swapChainImageView,
        .resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };
    vk::RenderingAttachmentInfo depthAttachmentInfo = {
        .imageView = renderTarget.depthImageView,
        .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = clearDepth
    };
    vk::RenderingInfo renderingInfo = {
        .renderArea = {.offset = {0, 0}, .extent = renderTarget.extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        .pDepthAttachment = &depthAttachmentInfo
    };

    commandBuffer_.beginRendering(renderingInfo);
    commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    commandBuffer_.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(renderTarget.extent.width),
                                              static_cast<float>(renderTarget.extent.height), 0.0f, 1.0f));
    commandBuffer_.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), renderTarget.extent));

    // bind frame data
    commandBuffer_.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        0,
        {frameDescriptorSet},
        {}
    );

    vk::DescriptorSet currentTexture{nullptr};
    vk::Buffer currentVertexBuffer{nullptr};
    vk::Buffer currentIndexBuffer{nullptr};

    for (const auto* object : objects) {
        // bind texture
        vk::DescriptorSet textureDescriptor = object->getTexture().getDescriptorSet();
        if (textureDescriptor != currentTexture) {
            commandBuffer_.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                pipelineLayout,
                1,
                {textureDescriptor},
                {}
            );
            currentTexture = textureDescriptor;
        }

        // bind vertex buffer
        vk::Buffer vertexBuffer = *object->getMesh().getVertexBuffer().buffer;
        if (vertexBuffer != currentVertexBuffer) {
            commandBuffer_.bindVertexBuffers(0, vertexBuffer, {0});
            currentVertexBuffer = vertexBuffer;
        }

        // bind index buffer
        vk::Buffer indexBuffer = *object->getMesh().getIndexBuffer().buffer;
        if (indexBuffer != currentIndexBuffer) {
            commandBuffer_.bindIndexBuffer(indexBuffer, 0, object->getMesh().getIndexType());
            currentIndexBuffer = indexBuffer;
        }

        // bind object-specific data
        commandBuffer_.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout,
            2,
            {object->getDescriptorSet(frameIndex)},
            {}
        );
        commandBuffer_.drawIndexed(object->getMesh().getIndexAmount(), 1, 0, 0, 0);
    }

    commandBuffer_.endRendering();

    Image::transitionLayout(
        renderTarget.swapChainImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer_
    );
    commandBuffer_.end();
}

void FrameResources::updateFrameData(const FrameData& data) const {
    void* mapped = uniformBuffer_.memory.mapMemory(0, sizeof(FrameData));
    memcpy(mapped, &data, sizeof(FrameData));
    uniformBuffer_.memory.unmapMemory();
}
