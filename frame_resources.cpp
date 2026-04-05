#include "frame_resources.h"

#include "vulkan_device.h"

FrameResources::FrameResources(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                               vk::DescriptorSetLayout descriptorSetLayout)
{
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

void FrameResources::beginFrame(const vk::raii::Device& device)
{
    auto fenceResult = device.waitForFences(*inFlightFence, vk::True, UINT64_MAX);
    if (fenceResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("failed to wait for fence");
    }
    commandBuffer.reset();
}

void FrameResources::recordCommandBuffer(const vk::raii::Pipeline& graphicsPipeline,
                                         const vk::raii::PipelineLayout& pipelineLayout,
                                         vk::DescriptorSet frameDescriptorSet, vk::Image image,
                                         vk::ImageView imageView, uint32_t frameIndex,
                                         vk::Extent2D extent, const std::vector<Mesh>& meshes) const
{
    commandBuffer.begin({});

    transitionImageLayout(
        image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput
    );
    vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = imageView,
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };
    vk::RenderingInfo renderingInfo = {
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
    commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width),
                                              static_cast<float>(extent.height), 0.0f, 1.0f));
    commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *pipelineLayout,
        0,
        {frameDescriptorSet},
        {}
    );

    for (const auto& mesh : meshes)
    {
        commandBuffer.bindVertexBuffers(0, *mesh.getVertexBuffer().buffer, {0});
        commandBuffer.bindIndexBuffer(*mesh.getIndexBuffer().buffer, 0, mesh.getIndexType());
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            *pipelineLayout,
            1, // first set
            {mesh.getDescriptorSet(frameIndex)},
            {} // dynamic offsets
        );
        commandBuffer.drawIndexed(mesh.getIndexAmount(), 1, 0, 0, 0);
    }

    commandBuffer.endRendering();

    transitionImageLayout(
        image,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite,
        {},
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eBottomOfPipe
    );
    commandBuffer.end();
}

void FrameResources::transitionImageLayout(
    vk::Image image,
    vk::ImageLayout oldLayout,
    vk::ImageLayout newLayout,
    vk::AccessFlags2 srcAccessMask,
    vk::AccessFlags2 dstAccessMask,
    vk::PipelineStageFlags2 srcStageMask,
    vk::PipelineStageFlags2 dstStageMask
) const
{
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vk::DependencyInfo dependencyInfo = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

void FrameResources::updateFrameData(const FrameData& data) const
{
    void* mapped = uniformBuffer.memory.mapMemory(0, sizeof(FrameData));
    memcpy(mapped, &data, sizeof(FrameData));
    uniformBuffer.memory.unmapMemory();
}
