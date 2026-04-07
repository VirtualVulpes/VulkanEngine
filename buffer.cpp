#include "buffer.h"
#include "vulkan_device.h"

namespace {
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties,
                            const vk::raii::PhysicalDevice& physicalDevice) {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type");
    }
}

namespace Buffer {
    BufferAllocation createAllocation(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                      vk::MemoryPropertyFlags properties,
                                      const VulkanDevice& device) {
        BufferAllocation buffer;

        vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
        buffer.buffer = vk::raii::Buffer(device.getDevice(), bufferInfo);
        vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice())
        };
        buffer.memory = vk::raii::DeviceMemory(device.getDevice(), allocInfo);
        buffer.buffer.bindMemory(*buffer.memory, 0);

        return buffer;
    }

    MappedBuffer createMapped(vk::DeviceSize size, vk::BufferUsageFlags usage, const VulkanDevice& device) {
        MappedBuffer buffer;

        vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
        buffer.buffer = vk::raii::Buffer(device.getDevice(), bufferInfo);
        vk::MemoryRequirements memRequirements = buffer.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent, device.getPhysicalDevice())
        };
        buffer.memory = vk::raii::DeviceMemory(device.getDevice(), allocInfo);
        buffer.buffer.bindMemory(*buffer.memory, 0);
        buffer.mappedPtr = buffer.memory.mapMemory(0, size);

        return buffer;
    }

    BufferAllocation createWithStaging(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage,
                                       const VulkanDevice& device) {
        auto stagingBuffer = createAllocation(size, vk::BufferUsageFlagBits::eTransferSrc,
                                              vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent, device);

        void* dataStaging = stagingBuffer.memory.mapMemory(0, size);
        memcpy(dataStaging, data, size);
        stagingBuffer.memory.unmapMemory();

        auto buffer = createAllocation(size, vk::BufferUsageFlagBits::eTransferDst | usage,
                                       vk::MemoryPropertyFlagBits::eDeviceLocal, device);

        copy(stagingBuffer.buffer, buffer.buffer, size, device);

        return buffer;
    }

    void copy(vk::Buffer source, vk::Buffer destination, vk::DeviceSize size,
              const VulkanDevice& device) {
        vk::CommandBufferAllocateInfo allocInfo{
            .commandPool = device.getCommandPool(), .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1
        };
        vk::raii::CommandBuffer commandCopyBuffer = std::move(
            device.getDevice().allocateCommandBuffers(allocInfo).front());

        commandCopyBuffer.begin(vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        commandCopyBuffer.copyBuffer(source, destination, vk::BufferCopy(0, 0, size));
        commandCopyBuffer.end();

        device.getQueue().submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer},
                                 nullptr);

        device.getQueue().waitIdle();
    }
}
