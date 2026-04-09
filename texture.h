#pragma once

#include "image.h"

namespace Assets {
    struct TextureData;
}

class Texture {
public:
    Texture(const VulkanDevice& device, const Assets::TextureData& data, const ImageConfig& config, vk::Sampler sampler, vk::raii::DescriptorSet descriptorSet);
    vk::DescriptorSet getDescriptorSet() const { return descriptor; }
private:
    ImageResource image;
    vk::Sampler sampler{nullptr};
    vk::raii::DescriptorSet descriptor;
};
