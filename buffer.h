#pragma once

class VulkanDevice;

struct MappedBuffer {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};
    void* mappedPtr{nullptr};
};

struct BufferAllocation {
    vk::raii::Buffer buffer{nullptr};
    vk::raii::DeviceMemory memory{nullptr};

    void copyData(vk::DeviceSize size, const void* data) const {
        void* address = memory.mapMemory(0, size);
        memcpy(address, data, size);
        memory.unmapMemory();
    }
};

namespace Buffer {
    BufferAllocation createAllocation(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                      vk::MemoryPropertyFlags properties,
                                      const VulkanDevice& device);

    MappedBuffer createMapped(vk::DeviceSize size, vk::BufferUsageFlags usage, const VulkanDevice& device);

    BufferAllocation createWithStaging(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage,
                                       const VulkanDevice& device);

    void copy(vk::Buffer source, vk::Buffer destination, vk::DeviceSize size,
              const VulkanDevice& device);
}
