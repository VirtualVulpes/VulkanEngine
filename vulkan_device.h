#pragma once

#include "window.h"

class VulkanDevice {
public:
    VulkanDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

    const vk::raii::Device& getDevice() const { return device; }
    const vk::raii::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
    const vk::raii::Queue& getQueue() const { return graphicsQueue; }
    uint32_t getQueueIndex() const { return queueIndex; }
    const vk::raii::CommandPool& getCommandPool() const { return commandPool; }
    const vk::raii::Sampler& getTextureSampler() const { return textureSampler; }

    vk::raii::CommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

private:
    void findPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

    void createLogicalDevice();

    void createCommandPool();

    void createTextureSampler();

    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};
    vk::raii::Queue graphicsQueue{nullptr};
    uint32_t queueIndex{VK_QUEUE_FAMILY_IGNORED};
    vk::raii::CommandPool commandPool{nullptr};

    vk::raii::Sampler textureSampler{nullptr};
};
