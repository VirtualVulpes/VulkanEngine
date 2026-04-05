#pragma once

#include "vulkan_device.h"
#include "window.h"

class SwapChain
{
public:
    SwapChain(const vk::raii::SurfaceKHR& surface, vk::Extent2D framebufferSize, const VulkanDevice& device);

    const vk::raii::SwapchainKHR& getSwapChain() const { return swapChain; }
    const vk::Image& getImage(uint32_t index) const { return images[index]; }
    uint32_t getImageSize() const { return images.size(); }
    const vk::raii::ImageView& getImageView(uint32_t index) const { return imageViews[index]; }
    const vk::SurfaceFormatKHR& getSurfaceFormat() const { return surfaceFormat; }
    const vk::Extent2D& getExtent() const { return extent; }

    void recreate(const Window& window, const VulkanDevice& device);

private:
    void create(const vk::raii::SurfaceKHR& surface, vk::Extent2D framebufferSize, const VulkanDevice& device);
    void createImageViews(const VulkanDevice& device);

    vk::raii::SwapchainKHR swapChain{nullptr};
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> imageViews;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;
};
