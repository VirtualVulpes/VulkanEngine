#include "image.h"

#include "buffer.h"
#include "memory.h"
#include "vulkan_device.h"
#include "ktx.h"

namespace {
    vk::raii::Image createImage(const VulkanDevice& device, uint32_t width, uint32_t height, vk::Format format,
                                vk::ImageTiling tiling, vk::ImageUsageFlags usage, uint32_t mipLevels = 1,
                                vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1) {
        vk::ImageCreateInfo imageInfo{
            .imageType = vk::ImageType::e2D,
            .format = format,
            .extent = {width, height, 1},
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = samples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = vk::SharingMode::eExclusive,
            .initialLayout = vk::ImageLayout::eUndefined
        };
        return {device.getDevice(), imageInfo};
    }

    vk::raii::DeviceMemory createMemory(const VulkanDevice& device, const vk::raii::Image& image,
                                        vk::MemoryPropertyFlags properties) {
        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = Memory::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits,
                                                      properties)
        };
        return {device.getDevice(), allocInfo};
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
                                 vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,
                                 vk::SampleCountFlagBits samples) {
        ImageResource resource;

        resource.image = createImage(device, width, height, format, tiling, usage, mipLevels, samples);
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

    ImageResource createTextureImage(std::string_view path, const VulkanDevice& device) {
        auto commandBuffer = device.beginSingleTimeCommands();

        ktxTexture* kTexture;
        KTX_error_code result = ktxTexture_CreateFromNamedFile(path.data(),
                                                               KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);

        if (result != KTX_SUCCESS) {
            throw std::runtime_error("failed to load ktx texture image");
        }

        // transcode and get format
        assert(kTexture->classId == ktxTexture2_c);

        auto* ktx2 = reinterpret_cast<ktxTexture2 *>(kTexture);
        if (ktxTexture2_NeedsTranscoding(ktx2)) {
            ktxTexture2_TranscodeBasis(ktx2, KTX_TTF_BC7_RGBA, 0);
        }

        auto textureFormat = static_cast<vk::Format>(ktx2->vkFormat);
        if (textureFormat == vk::Format::eUndefined) {
            textureFormat = vk::Format::eR8G8B8A8Unorm;
        }

        // get texture dimensions and data
        uint32_t textureWidth = kTexture->baseWidth;
        uint32_t textureHeight = kTexture->baseHeight;
        ktx_size_t textureSize = ktxTexture_GetDataSize(kTexture);
        ktx_uint8_t* textureData = ktxTexture_GetData(kTexture);

        // create staging buffer and copy data
        auto buffer = Buffer::createAllocation(textureSize, vk::BufferUsageFlagBits::eTransferSrc,
                                               vk::MemoryPropertyFlagBits::eHostVisible |
                                               vk::MemoryPropertyFlagBits::eHostCoherent, device);
        buffer.copyData(textureSize, textureData);

        // create the texture image
        uint32_t mipLevels = kTexture->numLevels;
        ImageResource textureImage = createResource(device, textureWidth, textureHeight, textureFormat,
                                                    vk::ImageTiling::eOptimal,
                                                    vk::ImageUsageFlagBits::eTransferDst |
                                                    vk::ImageUsageFlagBits::eSampled,
                                                    vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                    vk::ImageAspectFlagBits::eColor, mipLevels);

        // copy data from staging buffer to texture image
        transitionLayout(textureImage.image,
                         vk::ImageLayout::eUndefined,
                         vk::ImageLayout::eTransferDstOptimal,
                         {},
                         vk::AccessFlagBits2::eTransferWrite,
                         vk::PipelineStageFlagBits2::eTopOfPipe,
                         vk::PipelineStageFlagBits2::eTransfer,
                         vk::ImageAspectFlagBits::eColor,
                         commandBuffer
        );

        uint32_t mipWidth = textureWidth;
        uint32_t mipHeight = textureHeight;
        for (uint32_t i = 0; i < mipLevels; i++) {
            ktx_size_t offset;
            ktxTexture_GetImageOffset(kTexture, i, 0, 0, &offset);
            copyBufferToImage(commandBuffer, buffer.buffer, textureImage.image, mipWidth, mipHeight, offset, i);
            mipWidth = std::max(1u, mipWidth >> 1);
            mipHeight = std::max(1u, mipHeight >> 1);
        }

        transitionLayout(textureImage.image,
                         vk::ImageLayout::eTransferDstOptimal,
                         vk::ImageLayout::eShaderReadOnlyOptimal,
                         vk::AccessFlagBits2::eTransferWrite,
                         vk::AccessFlagBits2::eShaderRead,
                         vk::PipelineStageFlagBits2::eTransfer,
                         vk::PipelineStageFlagBits2::eFragmentShader,
                         vk::ImageAspectFlagBits::eColor,
                         commandBuffer
        );

        // cleanup resources
        ktxTexture_Destroy(kTexture);
        device.endSingleTimeCommands(commandBuffer);

        return textureImage;
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
