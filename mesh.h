#pragma once

#include "buffer.h"
#include "vertex.h"

class VulkanDevice;

class Mesh {
public:
    explicit Mesh(const VulkanDevice& device, const Assets::MeshData& meshData);

    const BufferAllocation& getVertexBuffer() const { return vertexBuffer_; }
    size_t getVertexAmount() const { return vertices_.size(); }
    const BufferAllocation& getIndexBuffer() const { return indexBuffer_; }
    size_t getIndexAmount() const { return indices_.size(); }
    vk::IndexType getIndexType() const { return vk::IndexTypeValue<decltype(indices_)::value_type>::value; }

private:
    void createBuffers(const VulkanDevice& device);

    std::vector<Vertex> vertices_{};
    std::vector<uint32_t> indices_{};

    BufferAllocation vertexBuffer_;
    BufferAllocation indexBuffer_;
};
