#include "mesh.h"

Mesh::Mesh(const VulkanDevice& device, const ModelData& modelData) : vertices(modelData.vertices), indices(modelData.indices) {
    vertexBuffer = Buffer::createWithStaging(vertices.data(), sizeof(vertices[0]) * vertices.size(),
                                             vk::BufferUsageFlagBits::eVertexBuffer, device);
    indexBuffer = Buffer::createWithStaging(indices.data(), sizeof(indices[0]) * indices.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer, device);
}

