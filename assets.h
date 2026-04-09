#pragma once

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
}
