#pragma once

#include "buffer.h"
#include "vertex.h"

class VulkanDevice;

class Mesh {
public:
    explicit Mesh(const VulkanDevice& device, const Assets::MeshData& meshData);

    const BufferAllocation& getVertexBuffer() const { return vertexBuffer; }
    size_t getVertexAmount() const { return vertices.size(); }
    const BufferAllocation& getIndexBuffer() const { return indexBuffer; }
    size_t getIndexAmount() const { return indices.size(); }
    vk::IndexType getIndexType() const { return vk::IndexTypeValue<decltype(indices)::value_type>::value; }

private:
    void createBuffers(const VulkanDevice& device);

    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    BufferAllocation vertexBuffer;
    BufferAllocation indexBuffer;
};
