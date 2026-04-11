#pragma once

#include <filesystem>

using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

namespace Config {
    // --- Rendering ---
    constexpr int MaxFramesInFlight = 2;

    // --- World ---
    constexpr uint32_t MaxObjects = 32;
    constexpr uint32_t MaxTextures = 32;
    constexpr auto WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // --- Window ---
    constexpr uint32_t WindowWidth = 800;
    constexpr uint32_t WindowHeight = 600;
    constexpr bool Fullscreen = false;

    // --- Vulkan Layers and Extensions ---
    constexpr std::array<const char *, 2> RequiredLayers{"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};
    constexpr std::array RequiredExtensions = {vk::KHRSwapchainExtensionName};

    // --- Paths ---
    inline const std::filesystem::path ModelPath{"models/"};
    inline const std::filesystem::path TexturePath{"textures/"};
    inline const std::filesystem::path ShaderPath{"shaders/"};

#ifdef NDEBUG
    constexpr bool EnableValidationLayers = false;
#else
    constexpr bool EnableValidationLayers = true;
#endif
}



