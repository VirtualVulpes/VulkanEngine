#include "frame_resources.h"

#include "image.h"
#include "vulkan_device.h"

FrameResources::FrameResources(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                               vk::DescriptorSetLayout descriptorSetLayout) {
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = device.getCommandPool(),
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    commandBuffer = std::move(vk::raii::CommandBuffers(device.getDevice(), allocateInfo).front());

    presentCompleteSemaphore = vk::raii::Semaphore(device.getDevice(), vk::SemaphoreCreateInfo());
    inFlightFence = vk::raii::Fence(device.getDevice(), {.flags = vk::FenceCreateFlagBits::eSignaled});

    uniformBuffer = Buffer::create(sizeof(FrameData),
                                   vk::BufferUsageFlagBits::eUniformBuffer,
                                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                   device);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout
    };
    descriptorSet = std::move(device.getDevice().allocateDescriptorSets(allocInfo).front());


    vk::DescriptorBufferInfo bufferInfo{
        .buffer = *uniformBuffer.buffer,
        .offset = 0,
        .range = sizeof(FrameData)
    };

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo
    };

    device.getDevice().updateDescriptorSets(descriptorWrite, {});
}

void FrameResources::beginFrame(const vk::raii::Device& device) {
    auto fenceResult = device.waitForFences(*inFlightFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess) {
        throw std::runtime_error("failed to wait for fence");
    }
    commandBuffer.reset();
}

void FrameResources::recordCommandBuffer(vk::Pipeline graphicsPipeline,
                                         vk::PipelineLayout pipelineLayout,
                                         vk::DescriptorSet frameDescriptorSet, RenderTarget renderTarget,
                                         uint32_t frameIndex,
                                         const std::vector<Mesh>& meshes) const {
    commandBuffer.begin({});

    Image::transitionLayout(
        renderTarget.colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer
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
        commandBuffer
    );

    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
    vk::RenderingAttachmentInfo colorAttachmentInfo = {
        .imageView = renderTarget.colorImageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
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

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(renderTarget.extent.width),
                                              static_cast<float>(renderTarget.extent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), renderTarget.extent));

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        0,
        {frameDescriptorSet},
        {}
    );

    for (const auto& mesh : meshes) {
        commandBuffer.bindVertexBuffers(0, *mesh.getVertexBuffer().buffer, {0});
        commandBuffer.bindIndexBuffer(*mesh.getIndexBuffer().buffer, 0, mesh.getIndexType());
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout,
            1, // first set
            {mesh.getDescriptorSet(frameIndex)},
            {} // dynamic offsets
        );
        commandBuffer.drawIndexed(mesh.getIndexAmount(), 1, 0, 0, 0);
    }

    commandBuffer.endRendering();

    Image::transitionLayout(
        renderTarget.colorImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor,
        commandBuffer
    );
    commandBuffer.end();
}

void FrameResources::updateFrameData(const FrameData& data) const {
    void* mapped = uniformBuffer.memory.mapMemory(0, sizeof(FrameData));
    memcpy(mapped, &data, sizeof(FrameData));
    uniformBuffer.memory.unmapMemory();
}
