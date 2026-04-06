#include "mesh.h"

#include "vulkan_device.h"

namespace {
    struct ObjectData {
        glm::mat4x4 model;
        uint32_t textureId;
    };
}

Mesh::Mesh(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
           vk::DescriptorSetLayout descriptorSetLayout, uint32_t textureIndex) {
    textureId = textureIndex;
    vertexBuffer = Buffer::createWithStaging(vertices.data(), sizeof(vertices[0]) * vertices.size(),
                                             vk::BufferUsageFlagBits::eVertexBuffer, device);
    indexBuffer = Buffer::createWithStaging(indices.data(), sizeof(indices[0]) * indices.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer, device);

    for (auto& buffer : objectBuffers) {
        buffer = Buffer::create(sizeof(ObjectData),
                                vk::BufferUsageFlagBits::eUniformBuffer,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                device);
    }

    std::vector layouts(Config::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };
    descriptorSets = device.getDevice().allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *objectBuffers[i].buffer,
            .offset = 0,
            .range = sizeof(ObjectData)
        };

        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo
        };

        device.getDevice().updateDescriptorSets(descriptorWrite, {});
    }
}

void Mesh::updateTransform() const {
    if (!transform.getIsDirty()) return;

    for (const auto& [buffer, memory] : objectBuffers) {
        ObjectData data{
            .model = transform.getModelMatrix(),
            .textureId = textureId
        };
        void* mapped = memory.mapMemory(0, sizeof(ObjectData));
        memcpy(mapped, &data, sizeof(ObjectData));
        memory.unmapMemory();
    }
}
