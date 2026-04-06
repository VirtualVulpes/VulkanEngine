#pragma once

class VulkanDevice;

struct ImageAllocation {
    vk::raii::Image image{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

struct ImageResource {
    vk::raii::Image image{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
    vk::raii::ImageView view{nullptr};
};

struct ImageHandle {
    vk::Image image{nullptr};
    vk::raii::ImageView view{nullptr};
};

namespace Image {
    ImageAllocation createAllocation(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                     vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                     vk::MemoryPropertyFlags properties);

    ImageResource createResource(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                 vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                                 vk::ImageAspectFlags);

    vk::raii::ImageView createView(const VulkanDevice& device, vk::Image image, vk::Format format,
                                   vk::ImageAspectFlags aspectFlags);

    void transitionLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                          vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
                          vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
                          vk::ImageAspectFlags imageAspectFlags, const vk::raii::CommandBuffer& commandBuffer
    );
};
