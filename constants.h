#pragma once

namespace Config {
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    constexpr std::array<const char *, 2> requiredLayers{"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };

    constexpr std::array requiredExtensions = {vk::KHRSwapchainExtensionName};

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
}



