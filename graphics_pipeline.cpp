#include "graphics_pipeline.h"

#include <fstream>

#include "mesh.h"
#include "swap_chain.h"

namespace
{
    std::vector<char> readFile(std::string_view filename)
    {
        std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file");
        }

        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        file.close();

        return buffer;
    }

    vk::raii::ShaderModule createShaderModule(const vk::raii::Device& device,
                                              const std::vector<char>& code)
    {
        vk::ShaderModuleCreateInfo createInfo{
            .codeSize = code.size() * sizeof(char), .pCode = reinterpret_cast<const uint32_t*>(code.data())
        };

        return {device, createInfo};
    }
}

GraphicsPipeline::GraphicsPipeline(const vk::raii::Device& device, vk::Format colorFormat,
                                   const vk::raii::DescriptorSetLayout& frameLayout,
                                   const vk::raii::DescriptorSetLayout& objectLayout)
{
    vk::raii::ShaderModule shaderModule = createShaderModule(device, readFile("shaders/slang.spv"));

    vk::PipelineShaderStageCreateInfo vertShaderCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shaderModule,
        .pName = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo fragShaderCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shaderModule,
        .pName = "fragMain"
    };

    vk::PipelineShaderStageCreateInfo shaderCreateInfo[]{
        vertShaderCreateInfo, fragShaderCreateInfo
    };

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vk::PipelineVertexInputStateCreateInfo vertexCreateInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{.topology = vk::PrimitiveTopology::eTriangleList};

    std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), .pDynamicStates = dynamicStates.data()
    };
    vk::PipelineViewportStateCreateInfo viewportCreateInfo{
        .viewportCount = 1, .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizationCreateInfo{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo{
        .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    std::array layouts = {*frameLayout, *objectLayout};
    vk::PipelineLayoutCreateInfo layoutCreateInfo{
        .setLayoutCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts  = layouts.data()};
    layout = vk::raii::PipelineLayout(device, layoutCreateInfo);

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> createInfo = {
        {
            .stageCount = 2,
            .pStages = shaderCreateInfo,
            .pVertexInputState = &vertexCreateInfo,
            .pInputAssemblyState = &inputAssemblyCreateInfo,
            .pViewportState = &viewportCreateInfo,
            .pRasterizationState = &rasterizationCreateInfo,
            .pMultisampleState = &multisampleCreateInfo,
            .pColorBlendState = &colorBlendingCreateInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = layout,
            .renderPass = nullptr
        },
        {.colorAttachmentCount = 1, .pColorAttachmentFormats = &colorFormat}
    };

    pipeline = vk::raii::Pipeline(device, nullptr, createInfo.get<vk::GraphicsPipelineCreateInfo>());
}
