#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "buffer.h"
#include "constants.h"
#include "transform.h"
#include "vertex.h"

class VulkanDevice;

class Mesh {
public:
    explicit Mesh(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
                  vk::DescriptorSetLayout descriptorSetLayout);

    const BufferAllocation& getVertexBuffer() const { return vertexBuffer; }
    size_t getVertexAmount() const { return vertices.size(); }
    const BufferAllocation& getIndexBuffer() const { return indexBuffer; }
    size_t getIndexAmount() const { return indices.size(); }
    vk::IndexType getIndexType() const { return vk::IndexTypeValue<decltype(indices)::value_type>::value; }
    const vk::raii::DescriptorSet& getDescriptorSet(uint32_t frameIndex) const { return descriptorSets[frameIndex]; }


    const Transform& getTransform() const { return transform; }
    Transform& getTransform() { return transform; }

    void updateTransform() const;

private:
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    BufferAllocation vertexBuffer;
    BufferAllocation indexBuffer;
    std::array<BufferAllocation, Config::MAX_FRAMES_IN_FLIGHT> transformBuffers;

    Transform transform{};
    std::vector<vk::raii::DescriptorSet> descriptorSets;
};
