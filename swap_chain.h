#pragma once
#include "image.h"

class Window;
class VulkanDevice;

class SwapChain {
public:
    SwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device);

    const vk::raii::SwapchainKHR& getSwapChain() const { return swapChain; }
    vk::Extent2D getExtent() const { return extent; }

    vk::Image getImage(uint32_t index) const { return images[index].image; }
    uint32_t getImageSize() const { return static_cast<uint32_t>(images.size()); }
    vk::ImageView getImageView(uint32_t index) const { return *images[index].view; }
    vk::Format getColorFormat() const { return surfaceFormat.format; }

    vk::Image getColorImage() const { return colorImage.image; }
    vk::ImageView getColorImageView() const { return colorImage.view; }

    vk::Image getDepthImage() const { return *depthImage.image; }
    vk::ImageView getDepthImageView() const { return *depthImage.view; }
    vk::Format getDepthFormat() const { return depthFormat; }

    const std::vector<ImageResource>& getTextureImages() { return textureImages; }

    void recreate(const Window& window, const VulkanDevice& device);

private:
    void createSwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device);

    void createColorResources(const VulkanDevice& device);

    void createDepthResources(const VulkanDevice& device);

    void createTextureResource(const VulkanDevice& device);

    vk::raii::SwapchainKHR swapChain{nullptr};
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;

    std::vector<ImageHandle> images;

    ImageResource colorImage{};
    ImageResource depthImage{};
    vk::Format depthFormat;

    std::vector<ImageResource> textureImages{};
};
