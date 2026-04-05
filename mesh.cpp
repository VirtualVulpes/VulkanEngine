#include "mesh.h"

#include "vulkan_device.h"

Mesh::Mesh(const VulkanDevice& device, vk::DescriptorPool descriptorPool,
           vk::DescriptorSetLayout descriptorSetLayout)
{
    vertexBuffer = Buffer::createWithStaging(vertices.data(), sizeof(vertices[0]) * vertices.size(),
                                             vk::BufferUsageFlagBits::eVertexBuffer, device);
    indexBuffer = Buffer::createWithStaging(indices.data(), sizeof(indices[0]) * indices.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer, device);

    for (auto& buffer : transformBuffers)
    {
        buffer = Buffer::create(sizeof(glm::mat4),
                                vk::BufferUsageFlagBits::eUniformBuffer,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                                device);
    }

    std::vector<vk::DescriptorSetLayout> layouts(Config::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };
    descriptorSets = device.getDevice().allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *transformBuffers[i].buffer,
            .offset = 0,
            .range = sizeof(glm::mat4)
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

void Mesh::updateTransform(uint32_t frameIndex) const
{
    glm::mat4 model = transform.getModelMatrix();
    void* mapped = transformBuffers[frameIndex].memory.mapMemory(0, sizeof(glm::mat4));
    memcpy(mapped, &model, sizeof(glm::mat4));
    transformBuffers[frameIndex].memory.unmapMemory();
}
