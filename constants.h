#pragma once

#include <array>
#include <cstdint>

namespace Config
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    constexpr std::array<const char*, 1> requiredLayers{
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
}



