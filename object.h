#pragma once

#include "constants.h"
#include "mesh.h"
#include "transform.h"

class Texture;

class Object {
public:
    Object(const VulkanDevice& device, const Transform& transform, const Mesh& mesh, const Assets::Material& material,
           std::vector<vk::raii::DescriptorSet> descriptorSets);

    const Transform& getTransform() const { return transform_; }
    Transform& getTransform() { return transform_; }

    const vk::raii::DescriptorSet& getDescriptorSet(uint32_t frameIndex) const { return descriptorSets_[frameIndex]; }

    const Mesh& getMesh() { return mesh_; }
    const Mesh& getMesh() const { return mesh_; }

    Assets::Material& getMaterial() { return material_; }
    const Assets::Material& getMaterial() const { return material_; }

    void updateTransform() const;

private:
    Transform transform_{};
    const Mesh& mesh_;
    Assets::Material material_;

    std::array<MappedBuffer, Config::MaxFramesInFlight> objectBuffers_;
    std::vector<vk::raii::DescriptorSet> descriptorSets_;
    void* mappedPtr_{nullptr};
};
