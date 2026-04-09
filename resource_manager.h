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

    Mesh& getMesh(const std::string& name) { return meshes.at(name); }

    Texture& getTexture(const std::string& name) { return textures.at(name); }

    vk::Sampler getSampler(const Assets::SamplerConfig& config);

    vk::DescriptorSetLayout getFrameLayout() { return frameLayout; }
    vk::DescriptorSetLayout getMaterialLayout() { return materialLayout; }
    vk::DescriptorSetLayout getObjectLayout() { return objectLayout; }
    vk::DescriptorPool getPool() { return descriptorPool; }
private:
    vk::raii::DescriptorSetLayout createFrameLayout();

    vk::raii::DescriptorSetLayout createMaterialLayout();

    vk::raii::DescriptorSetLayout createObjectLayout();

    vk::raii::DescriptorPool createDescriptorPool();

    vk::raii::Sampler createSampler(Assets::SamplerConfig config);

    const VulkanDevice& device;

    vk::raii::DescriptorPool descriptorPool;

    std::unordered_map<std::string, Mesh> meshes;
    std::unordered_map<std::string, Texture> textures;

    vk::raii::DescriptorSetLayout frameLayout;
    vk::raii::DescriptorSetLayout materialLayout;
    vk::raii::DescriptorSetLayout objectLayout;

    std::unordered_map<Assets::SamplerConfig, vk::raii::Sampler> textureSamplers;
};
