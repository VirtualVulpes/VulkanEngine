#include "image.h"

#include "memory.h"
#include "vulkan_device.h"

namespace {
    vk::raii::Image createImage(const VulkanDevice& device, uint32_t width, uint32_t height, uint32_t mipLevels,
                                const ImageConfig& config, vk::Format format,
                                vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1) {
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = samples,
            .tiling = config.tiling,
            .usage = config.usage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };
        return {device.getDevice(), imageInfo};
    }

    vk::raii::DeviceMemory createMemory(const VulkanDevice& device, const vk::raii::Image& image,
                                        vk::MemoryPropertyFlags memoryFlags) {
        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = Memory::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits,
                                                      memoryFlags)
        };
        return {device.getDevice(), allocInfo};
    }
}

namespace Image {
    ImageAllocation createAllocation(const VulkanDevice& device, uint32_t width, uint32_t height,
                                     const ImageConfig& config, vk::Format format) {
        ImageAllocation allocation{};

        allocation.image = createImage(device, width, height, 1, config, format);
        allocation.memory = createMemory(device, allocation.image, config.memory);
        allocation.image.bindMemory(allocation.memory, 0);

        return allocation;
    }

    ImageResource createResource(const VulkanDevice& device, uint32_t width, uint32_t height, uint32_t mipLevels,
                                 const ImageConfig& config, vk::Format format, vk::SampleCountFlagBits samples) {
        ImageResource resource;

        resource.image = createImage(device, width, height, mipLevels, config, format, samples);
        resource.memory = createMemory(device, resource.image, config.memory);
        resource.image.bindMemory(*resource.memory, 0);
        resource.view = createView(device, *resource.image, format, config.aspect);

        return resource;
    }

    vk::raii::ImageView createView(const VulkanDevice& device, vk::Image image, vk::Format format,
                                   vk::ImageAspectFlags aspectFlags) {
        vk::ImageViewCreateInfo createInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = format,
            .subresourceRange = {aspectFlags, 0, vk::RemainingMipLevels, 0, 1}
        };
        return {device.getDevice(), createInfo};
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
        const vk::raii::CommandBuffer& commandBuffer) {
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
                .levelCount = vk::RemainingMipLevels,
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

    void copyBufferToImage(const vk::raii::CommandBuffer& commandBuffer, const vk::raii::Buffer& buffer,
                       vk::raii::Image& image, uint32_t width, uint32_t height, uint32_t offset,
                       uint32_t mipLevel) {
        vk::BufferImageCopy region{
            .bufferOffset = offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {vk::ImageAspectFlagBits::eColor, mipLevel, 0, 1},
            .imageOffset = {0, 0, 0},
            .imageExtent = {width, height, 1}
        };
        commandBuffer.copyBufferToImage(*buffer, *image, vk::ImageLayout::eTransferDstOptimal, {region});
    }
}
