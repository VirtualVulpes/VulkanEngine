#pragma once
#include "image.h"

class Window;
class VulkanDevice;

class SwapChain {
public:
    SwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device);

    const vk::raii::SwapchainKHR& getSwapChain() const { return swapChain_; }
    vk::Extent2D getExtent() const { return extent_; }

    vk::Image getImage(uint32_t index) const { return images_[index].image; }
    uint32_t getImageSize() const { return static_cast<uint32_t>(images_.size()); }
    vk::ImageView getImageView(uint32_t index) const { return *images_[index].view; }
    vk::Format getColorFormat() const { return surfaceFormat_.format; }

    vk::Image getColorImage() const { return colorImage_.image; }
    vk::ImageView getColorImageView() const { return colorImage_.view; }

    vk::Image getDepthImage() const { return *depthImage_.image; }
    vk::ImageView getDepthImageView() const { return *depthImage_.view; }
    vk::Format getDepthFormat() const { return depthFormat_; }

    void recreate(const Window& window, const VulkanDevice& device);

private:
    void createSwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device);

    void createColorResources(const VulkanDevice& device);

    void createDepthResources(const VulkanDevice& device);

    vk::raii::SwapchainKHR swapChain_{nullptr};
    vk::SurfaceFormatKHR surfaceFormat_;
    vk::Extent2D extent_;

    std::vector<ImageHandle> images_;

    ImageResource colorImage_{};
    ImageResource depthImage_{};
    vk::Format depthFormat_;
};
