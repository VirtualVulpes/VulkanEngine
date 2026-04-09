#include "texture.h"

#include "buffer.h"

#include "assets.h"
#include "vulkan_device.h"

Texture::Texture(const VulkanDevice& device, const Assets::TextureData& data, const ImageConfig& config, vk::Sampler sampler, vk::raii::DescriptorSet descriptorSet) : sampler(sampler), descriptor(std::move(descriptorSet)) {
    image = Image::createResource(device, data.width, data.height, data.mipLevels, config, data.format);

    auto commandBuffer = device.beginSingleTimeCommands();

    // create staging buffer and copy data
    auto buffer = Buffer::createAllocation(data.pixels.size(), vk::BufferUsageFlagBits::eTransferSrc,
                                           vk::MemoryPropertyFlagBits::eHostVisible |
                                           vk::MemoryPropertyFlagBits::eHostCoherent, device);
    buffer.copyData(data.pixels.size(), data.pixels.data());

    // copy data from staging buffer to texture image
    Image::transitionLayout(image.image,
                            vk::ImageLayout::eUndefined,
                            vk::ImageLayout::eTransferDstOptimal,
                            {},
                            vk::AccessFlagBits2::eTransferWrite,
                            vk::PipelineStageFlagBits2::eTopOfPipe,
                            vk::PipelineStageFlagBits2::eTransfer,
                            config.aspect,
                            commandBuffer
    );

    uint32_t mipWidth = data.width;
    uint32_t mipHeight = data.height;
    for (uint32_t i = 0; i < data.mipLevels; i++) {
        Image::copyBufferToImage(commandBuffer, buffer.buffer, image.image, mipWidth, mipHeight, data.mipOffsets[i], i);
        mipWidth = std::max(1u, mipWidth >> 1);
        mipHeight = std::max(1u, mipHeight >> 1);
    }

    Image::transitionLayout(image.image,
                            vk::ImageLayout::eTransferDstOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::AccessFlagBits2::eTransferWrite,
                            vk::AccessFlagBits2::eShaderRead,
                            vk::PipelineStageFlagBits2::eTransfer,
                            vk::PipelineStageFlagBits2::eFragmentShader,
                            vk::ImageAspectFlagBits::eColor,
                            commandBuffer
    );

    device.endSingleTimeCommands(commandBuffer);

    vk::DescriptorImageInfo imageInfo{
        .sampler     = sampler,
        .imageView   = *image.view,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };

    vk::WriteDescriptorSet write{
        .dstSet          = *descriptor,
        .dstBinding      = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo      = &imageInfo
    };

    device.getDevice().updateDescriptorSets(write, nullptr);
}
