#pragma once

#include "window.h"

class VulkanDevice {
public:
    VulkanDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

    const vk::raii::Device& getDevice() const { return device_; }
    const vk::raii::PhysicalDevice& getPhysicalDevice() const { return physicalDevice_; }
    const vk::raii::Queue& getQueue() const { return graphicsQueue_; }
    uint32_t getQueueIndex() const { return queueIndex_; }
    const vk::raii::CommandPool& getCommandPool() const { return commandPool_; }
    vk::SampleCountFlagBits getMsaaSamples() const { return msaaSamples_; }

    vk::raii::CommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

private:
    void findPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface);

    void createLogicalDevice();

    void createCommandPool();

    vk::raii::PhysicalDevice physicalDevice_{nullptr};
    vk::raii::Device device_{nullptr};
    vk::raii::Queue graphicsQueue_{nullptr};
    uint32_t queueIndex_{VK_QUEUE_FAMILY_IGNORED};
    vk::raii::CommandPool commandPool_{nullptr};

    vk::SampleCountFlagBits msaaSamples_{vk::SampleCountFlagBits::e1};
};
