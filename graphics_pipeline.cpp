#include "graphics_pipeline.h"

#include <fstream>

#include "mesh.h"
#include "swap_chain.h"
#include "vulkan_device.h"

GraphicsPipeline::GraphicsPipeline(const VulkanDevice& device, Assets::PipelineConfig config,
                                   const Assets::Shader& shader,
                                   vk::DescriptorSetLayout frame, vk::DescriptorSetLayout material,
                                   vk::DescriptorSetLayout object) {
    vk::PipelineShaderStageCreateInfo vertShaderCreateInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = shader.vert,
        .pName = "vertMain"
    };

    vk::PipelineShaderStageCreateInfo fragShaderCreateInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = shader.frag,
        .pName = "fragMain"
    };

    std::array stages = {vertShaderCreateInfo, fragShaderCreateInfo};

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
        .polygonMode = config.polygonMode,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo{
        .rasterizationSamples = device.getMsaaSamples(), .sampleShadingEnable = vk::False
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

    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .depthBoundsTestEnable = vk::False,
        .stencilTestEnable = vk::False
    };

    std::array layouts = {frame, material, object};
    vk::PipelineLayoutCreateInfo layoutCreateInfo{
        .setLayoutCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };
    layout_ = vk::raii::PipelineLayout(device.getDevice(), layoutCreateInfo);

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> createInfo = {
        {
            .stageCount = 2,
            .pStages = stages.data(),
            .pVertexInputState = &vertexCreateInfo,
            .pInputAssemblyState = &inputAssemblyCreateInfo,
            .pViewportState = &viewportCreateInfo,
            .pRasterizationState = &rasterizationCreateInfo,
            .pMultisampleState = &multisampleCreateInfo,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlendingCreateInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = *layout_,
        },
        {
            .colorAttachmentCount = 1, .pColorAttachmentFormats = &config.colorFormat,
            .depthAttachmentFormat = config.depthFormat
        }
    };

    pipeline_ = vk::raii::Pipeline(device.getDevice(), nullptr, createInfo.get<vk::GraphicsPipelineCreateInfo>());
}
