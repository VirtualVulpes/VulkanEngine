#pragma once

class GraphicsPipeline
{
public:
    GraphicsPipeline(const vk::raii::Device& device, vk::Format colorFormat,
                     const vk::raii::DescriptorSetLayout& frameLayout,
                     const vk::raii::DescriptorSetLayout& objectLayout);
    const vk::raii::Pipeline& getPipeline() { return pipeline; }
    const vk::raii::PipelineLayout& getLayout() { return layout; }

private:
    vk::raii::PipelineLayout layout{nullptr};
    vk::raii::Pipeline pipeline{nullptr};
};
