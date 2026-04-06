#include "image.h"

#include "memory.h"
#include "vulkan_device.h"

namespace {
    vk::raii::Image createImage(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                vk::ImageTiling tiling, vk::ImageUsageFlags usage) {
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };
        return vk::raii::Image(device.getDevice(), imageInfo);
    }

    vk::raii::DeviceMemory createMemory(const VulkanDevice& device, const vk::raii::Image& image,
                                        vk::MemoryPropertyFlags properties) {
        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = Memory::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits,
                                                      properties)
        };
        return vk::raii::DeviceMemory(device.getDevice(), allocInfo);
    }
}

namespace Image {
    ImageAllocation createAllocation(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                     vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                                     vk::MemoryPropertyFlags properties) {
        ImageAllocation allocation{};

        allocation.image = createImage(device, width, height, format, tiling, usage);
        allocation.memory = createMemory(device, allocation.image, properties);
        allocation.image.bindMemory(allocation.memory, 0);

        return allocation;
    }

    ImageResource createResource(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                 vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                                 vk::ImageAspectFlags aspectFlags) {
        ImageResource resource;

        resource.image = createImage(device, width, height, format, tiling, usage);
        resource.memory = createMemory(device, resource.image, properties);
        resource.image.bindMemory(*resource.memory, 0);
        resource.view = createView(device, *resource.image, format, aspectFlags);

        return resource;
    }

    vk::raii::ImageView createView(const VulkanDevice& device, vk::Image image, vk::Format format,
                                   vk::ImageAspectFlags aspectFlags) {
        vk::ImageViewCreateInfo createInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .subresourceRange = {aspectFlags, 0, 1, 0, 1}
        };
        return vk::raii::ImageView(device.getDevice(), createInfo);
    }

    void transitionLayout(
        vk::Image image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        vk::ImageAspectFlags imageAspectFlags,
        const vk::raii::CommandBuffer& commandBuffer
    ) {
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
                .aspectMask = imageAspectFlags,
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
}
