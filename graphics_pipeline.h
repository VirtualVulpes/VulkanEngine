#pragma once

class GraphicsPipeline {
public:
    GraphicsPipeline(const vk::raii::Device& device, vk::Format colorFormat, vk::Format depthFormat,
                     vk::DescriptorSetLayout frameLayout, vk::DescriptorSetLayout objectLayout);

    const vk::raii::Pipeline& getPipeline() { return pipeline; }
    const vk::raii::PipelineLayout& getLayout() { return layout; }

private:
    vk::raii::PipelineLayout layout{nullptr};
    vk::raii::Pipeline pipeline{nullptr};
};
