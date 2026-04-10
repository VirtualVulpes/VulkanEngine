#pragma once

#include "mesh.h"

class Object;
class ImageResource;

struct RenderTarget {
    vk::Image swapChainImage;
    vk::ImageView swapChainImageView;
    vk::Image colorImage;
    vk::ImageView colorImageView;
    vk::Image depthImage;
    vk::ImageView depthImageView;
    vk::Extent2D extent;
};

struct FrameData {
    glm::mat4 view;
    glm::mat4 proj;
};

class FrameResources {
public:
    FrameResources(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                   vk::DescriptorSetLayout descriptorSetLayout);

    const vk::raii::CommandBuffer& getCommandBuffer() const { return commandBuffer_; }
    const vk::raii::Semaphore& getPresentCompleteSemaphore() const { return presentCompleteSemaphore_; }
    const vk::raii::Fence& getInFlightFence() const { return inFlightFence_; }
    const vk::raii::DescriptorSet& getDescriptorSet() const { return descriptorSet_; }

    void beginFrame(const vk::raii::Device& device);

    void recordCommandBuffer(vk::Pipeline graphicsPipeline,
                             vk::PipelineLayout pipelineLayout,
                             vk::DescriptorSet frameDescriptorSet,
                             RenderTarget renderTarget,
                             uint32_t frameIndex,
                             std::span<const Object* const> objects) const;

    void resetCommandBuffer() { commandBuffer_.reset(); }

    void updateFrameData(const FrameData& data) const;

private:
    vk::raii::CommandBuffer commandBuffer_{nullptr};
    vk::raii::Semaphore presentCompleteSemaphore_{nullptr};
    vk::raii::Fence inFlightFence_{nullptr};
    BufferAllocation uniformBuffer_;

    vk::raii::DescriptorSet descriptorSet_{nullptr};
};
