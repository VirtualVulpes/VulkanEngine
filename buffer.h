#pragma once

class VulkanDevice;

struct BufferAllocation
{
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

namespace Buffer
{
    BufferAllocation create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                            const VulkanDevice& device);
    BufferAllocation createWithStaging(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage,
                                       const VulkanDevice& device);
    void copy(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size,
              const VulkanDevice& device);
}
