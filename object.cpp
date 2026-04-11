#include "object.h"

#include "vulkan_device.h"

namespace {
    struct ObjectData {
        glm::mat4x4 model;
    };
}

Object::Object(const VulkanDevice& device, const Transform& transform, Mesh& mesh,
               Texture& texture, std::vector<vk::raii::DescriptorSet> descriptorSets) : transform_(transform), mesh_(mesh),
    texture_(texture), descriptorSets_(std::move(descriptorSets)) {
    for (auto& buffer : objectBuffers_) {
        buffer = Buffer::createMapped(sizeof(ObjectData), vk::BufferUsageFlagBits::eUniformBuffer, device);
    }

    for (size_t i = 0; i < Config::MaxFramesInFlight; i++) {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *objectBuffers_[i].buffer,
            .offset = 0,
            .range = sizeof(ObjectData)
        };

        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = descriptorSets_[i],
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
    if (!transform_.getIsDirty()) return;

    for (const auto& buffer : objectBuffers_) {
        ObjectData data{
            .model = transform_.getModelMatrix(),
        };
        memcpy(buffer.mappedPtr, &data, sizeof(ObjectData));
    }
}
