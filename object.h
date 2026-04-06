#pragma once

#include "constants.h"
#include "mesh.h"
#include "transform.h"

class Object {
public:
    Object(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
           vk::DescriptorSetLayout descriptorSetLayout, const Transform& transform, const Mesh& mesh,
           uint32_t textureId);

    const Transform& getTransform() const { return transform; }
    Transform& getTransform() { return transform; }
    const vk::raii::DescriptorSet& getDescriptorSet(uint32_t frameIndex) const { return descriptorSets[frameIndex]; }
    const Mesh& getMesh() const { return mesh; }

    void updateTransform() const;

private:
    Transform transform{};
    const Mesh& mesh;
    uint32_t textureId{};

    std::array<BufferAllocation, Config::MAX_FRAMES_IN_FLIGHT> objectBuffers;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
};
