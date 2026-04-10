#pragma once

#include <filesystem>

#include "mesh.h"
#include "texture.h"

class VulkanDevice;

class ResourceManager {
public:
    ResourceManager(const VulkanDevice& device);

    Mesh& loadMesh(std::string_view name, const Assets::MeshData& data);

    Mesh& loadMesh(std::string_view name);

    Texture& loadTexture(std::string_view name, const Assets::SamplerConfig& samplerConfig = Assets::SamplerConfig{});

    Mesh& getMesh(std::string_view name) { return meshes_.at(name.data()); }

    Texture& getTexture(std::string_view name) { return textures_.at(name.data()); }

    vk::Sampler getSampler(const Assets::SamplerConfig& config);

    vk::DescriptorSetLayout getFrameLayout() { return frameLayout_; }
    vk::DescriptorSetLayout getMaterialLayout() { return materialLayout_; }
    vk::DescriptorSetLayout getObjectLayout() { return objectLayout_; }
    vk::DescriptorPool getPool() { return descriptorPool_; }

private:
    vk::raii::DescriptorSetLayout createFrameLayout();

    vk::raii::DescriptorSetLayout createMaterialLayout();

    vk::raii::DescriptorSetLayout createObjectLayout();

    vk::raii::DescriptorPool createDescriptorPool();

    vk::raii::Sampler createSampler(Assets::SamplerConfig config);

    const VulkanDevice& device_;

    vk::raii::DescriptorPool descriptorPool_;

    std::unordered_map<std::string, Mesh> meshes_;
    std::unordered_map<std::string, Texture> textures_;

    vk::raii::DescriptorSetLayout frameLayout_;
    vk::raii::DescriptorSetLayout materialLayout_;
    vk::raii::DescriptorSetLayout objectLayout_;

    std::unordered_map<Assets::SamplerConfig, vk::raii::Sampler> textureSamplers_;
};
