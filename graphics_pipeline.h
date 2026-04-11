#pragma once

namespace Assets {
    struct PipelineConfig;
    struct Shader;
}

class VulkanDevice;

class GraphicsPipeline {
public:
    GraphicsPipeline(const VulkanDevice& device, Assets::PipelineConfig config, const Assets::Shader& shader,
                     vk::DescriptorSetLayout frame, vk::DescriptorSetLayout material, vk::DescriptorSetLayout object);

    const vk::raii::Pipeline& getPipeline() const { return pipeline_; }
    const vk::raii::PipelineLayout& getLayout() const { return layout_; }

private:
    vk::raii::PipelineLayout layout_{nullptr};
    vk::raii::Pipeline pipeline_{nullptr};
};
