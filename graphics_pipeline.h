#pragma once

class VulkanDevice;

class GraphicsPipeline {
public:
    GraphicsPipeline(const VulkanDevice& device, vk::Format colorFormat, vk::Format depthFormat,
                     vk::DescriptorSetLayout frameLayout, vk::DescriptorSetLayout textureLayout, vk::DescriptorSetLayout objectLayout);

    const vk::raii::Pipeline& getPipeline() { return pipeline_; }
    const vk::raii::PipelineLayout& getLayout() { return layout_; }

private:
    vk::raii::PipelineLayout layout_{nullptr};
    vk::raii::Pipeline pipeline_{nullptr};
};
