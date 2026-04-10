#pragma once

#include "image.h"

namespace Assets {
    struct TextureData;
}

class Texture {
public:
    Texture(const VulkanDevice& device, const Assets::TextureData& data, const ImageConfig& config, vk::raii::DescriptorSet descriptorSet);
    vk::DescriptorSet getDescriptorSet() const { return descriptor_; }
private:
    ImageResource image_;
    vk::raii::DescriptorSet descriptor_;
};
