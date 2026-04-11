#pragma once

#include <filesystem>

#include "mesh.h"
#include "texture.h"
#include "graphics_pipeline.h"

class VulkanDevice;

class ResourceManager {
public:
    ResourceManager(const VulkanDevice& device);

    Mesh& loadMesh(std::string_view name, const Assets::MeshData& data);

    Mesh& loadMesh(std::string_view name, std::string_view fileName);

    Texture& loadTexture(std::string_view name, std::string_view fileName,
                         const Assets::SamplerConfig& samplerConfig = Assets::SamplerConfig{});

    vk::raii::ShaderModule& loadShaderModule(std::string_view name, std::string_view fileName);

    Assets::Shader& createShader(std::string_view name, std::string_view vertName, std::string_view fragName);

    Assets::Material& createMaterial(std::string_view name, std::string_view pipeline, std::string_view texture);

    const GraphicsPipeline& createPipeline(std::string_view name, const Assets::PipelineConfig& config,
                                           std::string_view shader);

    Mesh& getMesh(std::string_view name) { return meshes_.at(name.data()); }
    const Mesh& getMesh(std::string_view name) const { return meshes_.at(name.data()); }

    Texture& getTexture(std::string_view name) { return textures_.at(name.data()); }
    const Texture& getTexture(std::string_view name) const { return textures_.at(name.data()); }

    Assets::Shader& getShader(std::string_view name) {
        return shaders_.at(name.data());
    }

    const Assets::Shader& getShader(std::string_view name) const {
        return shaders_.at(name.data());
    }

    Assets::Material& getMaterial(std::string_view name) { return materials_.at(name.data()); }
    const Assets::Material& getMaterial(std::string_view name) const { return materials_.at(name.data()); }

    const GraphicsPipeline& getPipeline(std::string_view name) const {
        return pipelines_.at(name.data());
    }

    const std::unordered_map<std::string, GraphicsPipeline>& getPipelines() const {
        return pipelines_;
    }

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
    std::unordered_map<std::string, Assets::Shader> shaders_;
    std::unordered_map<std::string, vk::raii::ShaderModule> shaderModules_;
    std::unordered_map<std::string, Assets::Material> materials_;

    vk::raii::DescriptorSetLayout frameLayout_;
    vk::raii::DescriptorSetLayout materialLayout_;
    vk::raii::DescriptorSetLayout objectLayout_;

    std::unordered_map<Assets::SamplerConfig, vk::raii::Sampler> textureSamplers_;
    std::unordered_map<std::string, GraphicsPipeline> pipelines_;
};
