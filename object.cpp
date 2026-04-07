#include "object.h"

#include "vulkan_device.h"


namespace {
    struct ObjectData {
        glm::mat4x4 model;
        uint32_t textureId;
    };
}

Object::Object(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
               vk::DescriptorSetLayout descriptorSetLayout, const Transform& transform, const Mesh& mesh,
               uint32_t textureId) : transform(transform), mesh(mesh),
                                     textureId(textureId) {
    for (auto& buffer : objectBuffers) {
        buffer = Buffer::createMapped(sizeof(ObjectData), vk::BufferUsageFlagBits::eUniformBuffer, device);
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

    updateTransform();
}

void Object::updateTransform() const {
    if (!transform.getIsDirty()) return;

    for (const auto& buffer : objectBuffers) {
        ObjectData data{
            .model = transform.getModelMatrix(),
            .textureId = textureId
        };
        memcpy(buffer.mappedPtr, &data, sizeof(ObjectData));
    }
}
