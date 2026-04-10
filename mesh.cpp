#include "mesh.h"

Mesh::Mesh(const VulkanDevice& device, const Assets::MeshData& meshData)
    : vertices_(meshData.vertices), indices_(meshData.indices) {
    createBuffers(device);
}

void Mesh::createBuffers(const VulkanDevice& device) {
    vertexBuffer_ = Buffer::createWithStaging(vertices_.data(), sizeof(vertices_[0]) * vertices_.size(),
                                             vk::BufferUsageFlagBits::eVertexBuffer, device);
    indexBuffer_ = Buffer::createWithStaging(indices_.data(), sizeof(indices_[0]) * indices_.size(),
                                            vk::BufferUsageFlagBits::eIndexBuffer, device);
}