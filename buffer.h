#pragma once

class VulkanDevice;

struct BufferAllocation {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
};

namespace Buffer {
    BufferAllocation create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                            const VulkanDevice& device);

    BufferAllocation createWithStaging(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage,
                                       const VulkanDevice& device);

    void copy(vk::Buffer source, vk::Buffer destination, vk::DeviceSize size,
              const VulkanDevice& device);
}
