#pragma once

namespace Memory {
    uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter,
                                   vk::MemoryPropertyFlags properties);
}
