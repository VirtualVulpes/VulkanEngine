#pragma once

#include "assets.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = vk::VertexInputRate::eVertex
        };
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
        return {
            {
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
                {
                    .location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, color)
                },
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, uv)}
            }
        };
    }
};

namespace Shape {
    inline Assets::MeshData plane() {
        return {
            .vertices = {
                {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
                {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
                {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
                {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
            },
            .indices = {
                0, 1, 2, 2, 3, 0
            }
        };
    }

    inline Assets::MeshData cube() {
        return {
            .vertices = {
                // Front (+Z)
                {{-0.5f, -0.5f, 0.5f}, {1, 1, 1}, {0, 0}},
                {{0.5f, -0.5f, 0.5f}, {1, 1, 1}, {1, 0}},
                {{0.5f, 0.5f, 0.5f}, {1, 1, 1}, {1, 1}},
                {{-0.5f, 0.5f, 0.5f}, {1, 1, 1}, {0, 1}},

                // Back (-Z)
                {{0.5f, -0.5f, -0.5f}, {1, 1, 1}, {0, 0}},
                {{-0.5f, -0.5f, -0.5f}, {1, 1, 1}, {1, 0}},
                {{-0.5f, 0.5f, -0.5f}, {1, 1, 1}, {1, 1}},
                {{0.5f, 0.5f, -0.5f}, {1, 1, 1}, {0, 1}},

                // Left (-X)
                {{-0.5f, -0.5f, -0.5f}, {1, 1, 1}, {0, 0}},
                {{-0.5f, -0.5f, 0.5f}, {1, 1, 1}, {1, 0}},
                {{-0.5f, 0.5f, 0.5f}, {1, 1, 1}, {1, 1}},
                {{-0.5f, 0.5f, -0.5f}, {1, 1, 1}, {0, 1}},

                // Right (+X)
                {{0.5f, -0.5f, 0.5f}, {1, 1, 1}, {0, 0}},
                {{0.5f, -0.5f, -0.5f}, {1, 1, 1}, {1, 0}},
                {{0.5f, 0.5f, -0.5f}, {1, 1, 1}, {1, 1}},
                {{0.5f, 0.5f, 0.5f}, {1, 1, 1}, {0, 1}},

                // Top (+Y)
                {{-0.5f, 0.5f, 0.5f}, {1, 1, 1}, {0, 0}},
                {{0.5f, 0.5f, 0.5f}, {1, 1, 1}, {1, 0}},
                {{0.5f, 0.5f, -0.5f}, {1, 1, 1}, {1, 1}},
                {{-0.5f, 0.5f, -0.5f}, {1, 1, 1}, {0, 1}},

                // Bottom (-Y)
                {{-0.5f, -0.5f, -0.5f}, {1, 1, 1}, {0, 0}},
                {{0.5f, -0.5f, -0.5f}, {1, 1, 1}, {1, 0}},
                {{0.5f, -0.5f, 0.5f}, {1, 1, 1}, {1, 1}},
                {{-0.5f, -0.5f, 0.5f}, {1, 1, 1}, {0, 1}},
            },
            .indices = {
                0, 1, 2, 2, 3, 0, // Front
                4, 5, 6, 6, 7, 4, // Back
                8, 9, 10, 10, 11, 8, // Left
                12, 13, 14, 14, 15, 12, // Right
                16, 17, 18, 18, 19, 16, // Top
                20, 21, 22, 22, 23, 20 // Bottom
            }
        };
    }
}
