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
    BufferAllocation create(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                            const VulkanDevice& device) {
        BufferAllocation allocation;

        vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
        allocation.buffer = vk::raii::Buffer(device.getDevice(), bufferInfo);
        vk::MemoryRequirements memRequirements = allocation.buffer.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, device.getPhysicalDevice())
        };
        allocation.memory = vk::raii::DeviceMemory(device.getDevice(), allocInfo);
        allocation.buffer.bindMemory(*allocation.memory, 0);

        return allocation;
    }

    BufferAllocation createWithStaging(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage,
                                       const VulkanDevice& device) {
        auto stagingBuffer = create(size, vk::BufferUsageFlagBits::eTransferSrc,
                                    vk::MemoryPropertyFlagBits::eHostVisible |
                                    vk::MemoryPropertyFlagBits::eHostCoherent, device);

        void* dataStaging = stagingBuffer.memory.mapMemory(0, size);
        memcpy(dataStaging, data, size);
        stagingBuffer.memory.unmapMemory();

        auto buffer = create(size, vk::BufferUsageFlagBits::eTransferDst | usage,
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
