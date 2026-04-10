#pragma once

#include <filesystem>

namespace Config {
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    constexpr uint32_t MAX_OBJECTS = 32;
    constexpr uint32_t MAX_TEXTURES = 32;

    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    constexpr auto UP_DIRECTION = glm::vec3(0.0f, 1.0f, 0.0f);

    constexpr std::array<const char *, 2> requiredLayers{"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"};

    constexpr std::array requiredExtensions = {vk::KHRSwapchainExtensionName};

    inline const std::filesystem::path modelPath{"models/"};
    inline const std::filesystem::path texturePath{"textures/"};

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
}



