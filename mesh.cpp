#include "mesh.h"

Mesh::Mesh(const VulkanDevice& device, const Assets::MeshData& meshData)
    : vertices(meshData.vertices), indices(meshData.indices) {
    createBuffers(device);
}

void Mesh::createBuffers(const VulkanDevice& device) {
    vertexBuffer = Buffer::createWithStaging(vertices.data(), sizeof(vertices[0]) * vertices.size(),
                                             vk::BufferUsageFlagBits::eVertexBuffer, device);
    indexBuffer = Buffer::createWithStaging(indices.data(), sizeof(indices[0]) * indices.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer, device);
}