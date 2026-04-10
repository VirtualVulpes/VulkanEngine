#pragma once

#include "constants.h"
#include "mesh.h"
#include "transform.h"

class Texture;

class Object {
public:
    Object(const VulkanDevice& device, const Transform& transform, Mesh& mesh, Texture& texture,
           std::vector<vk::raii::DescriptorSet> descriptorSets);

    const Transform& getTransform() const { return transform_; }
    Transform& getTransform() { return transform_; }
    const vk::raii::DescriptorSet& getDescriptorSet(uint32_t frameIndex) const { return descriptorSets_[frameIndex]; }
    Mesh& getMesh() { return mesh_; }
    const Mesh& getMesh() const { return mesh_; }
    Texture& getTexture() { return texture_; }
    const Texture& getTexture() const { return texture_; }

    void updateTransform() const;

private:
    Transform transform_{};
    Mesh& mesh_;
    Texture& texture_;

    std::array<MappedBuffer, Config::MAX_FRAMES_IN_FLIGHT> objectBuffers_;
    std::vector<vk::raii::DescriptorSet> descriptorSets_;
    void* mappedPtr_{nullptr};
};
