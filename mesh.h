#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "buffer.h"
#include "vertex.h"

class VulkanDevice;

class Mesh {
public:
    explicit Mesh(const VulkanDevice& device, const ModelData& modelData);

    const BufferAllocation& getVertexBuffer() const { return vertexBuffer; }
    size_t getVertexAmount() const { return vertices.size(); }
    const BufferAllocation& getIndexBuffer() const { return indexBuffer; }
    size_t getIndexAmount() const { return indices.size(); }
    vk::IndexType getIndexType() const { return vk::IndexTypeValue<decltype(indices)::value_type>::value; }

private:
    const std::vector<Vertex> vertices{};

    const std::vector<uint32_t> indices{};

    BufferAllocation vertexBuffer;
    BufferAllocation indexBuffer;
};
