#pragma once

#include "mesh.h"

struct FrameData
{
    glm::mat4 view;
    glm::mat4 proj;
};

class FrameResources
{
public:
    FrameResources(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                   vk::DescriptorSetLayout descriptorSetLayout);

    const vk::raii::CommandBuffer& getCommandBuffer() const { return commandBuffer; }
    const vk::raii::Semaphore& getPresentCompleteSemaphore() const { return presentCompleteSemaphore; }
    const vk::raii::Fence& getInFlightFence() const { return inFlightFence; }
    const vk::raii::DescriptorSet& getDescriptorSet() const { return descriptorSet; }

    void beginFrame(const vk::raii::Device& device);
    void recordCommandBuffer(const vk::raii::Pipeline& graphicsPipeline,
                             const vk::raii::PipelineLayout& pipelineLayout,
                             vk::DescriptorSet frameDescriptorSet, vk::Image image,
                             vk::ImageView imageView, uint32_t frameIndex,
                             vk::Extent2D extent, const std::vector<Mesh>& meshes) const;
    void resetCommandBuffer() { commandBuffer.reset(); }
    void updateFrameData(const FrameData& data) const;

private:
    void transitionImageLayout(
        vk::Image image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask
    ) const;

    vk::raii::CommandBuffer commandBuffer{nullptr};
    vk::raii::Semaphore presentCompleteSemaphore{nullptr};
    vk::raii::Fence inFlightFence{nullptr};
    BufferAllocation uniformBuffer;

    vk::raii::DescriptorSet descriptorSet{nullptr};
};
