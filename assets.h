#pragma once

#include "utils.h"

struct Vertex;

namespace Assets {
    struct MeshData {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    struct TextureData {
        std::vector<std::byte> pixels;
        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;
        std::vector<size_t> mipOffsets;
        vk::Format format;
    };

    struct SamplerConfig {
        vk::Filter filter{vk::Filter::eLinear};
        vk::SamplerMipmapMode mipmapMode{vk::SamplerMipmapMode::eLinear};
        vk::SamplerAddressMode addressMode{vk::SamplerAddressMode::eRepeat};
        bool anisotropy{true};

        bool operator==(const SamplerConfig&) const = default;
    };
}

template<>
struct std::hash<Assets::SamplerConfig> {
    size_t operator()(const Assets::SamplerConfig& c) const {
        size_t h = 0;
        Utils::hashCombine(h, static_cast<int>(c.filter));
        Utils::hashCombine(h, static_cast<int>(c.mipmapMode));
        Utils::hashCombine(h, static_cast<int>(c.addressMode));
        Utils::hashCombine(h, c.anisotropy);
        return h;
    }
};
