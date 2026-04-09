#pragma once

#include "constants.h"
#include "mesh.h"
#include "transform.h"

class Texture;

class Object {
public:
    Object(const VulkanDevice& device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout descriptorSetLayout,
           const Transform& transform, Mesh& mesh, Texture& texture);

    const Transform& getTransform() const { return transform; }
    Transform& getTransform() { return transform; }
    const vk::raii::DescriptorSet& getDescriptorSet(uint32_t frameIndex) const { return descriptorSets[frameIndex]; }
    Mesh& getMesh() { return mesh; }
    const Mesh& getMesh() const { return mesh; }
    Texture& getTexture() { return texture; }
    const Texture& getTexture() const { return texture; }

    void updateTransform() const;

private:
    Transform transform{};
    Mesh& mesh;
    Texture& texture;

    std::array<MappedBuffer, Config::MAX_FRAMES_IN_FLIGHT> objectBuffers;
    std::vector<vk::raii::DescriptorSet> descriptorSets;
    void* mappedPtr{nullptr};
};
