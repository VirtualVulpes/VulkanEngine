#include "swap_chain.h"

#include "image.h"
#include "vulkan_device.h"
#include "window.h"

namespace {
    uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        if ((surfaceCapabilities.maxImageCount > 0) && (surfaceCapabilities.maxImageCount < minImageCount)) {
            minImageCount = surfaceCapabilities.maxImageCount;
        }
        return minImageCount;
    }

    vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        assert(!availableFormats.empty());

        const auto formatIt = std::ranges::find_if(availableFormats, [&](const auto& format) {
            return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

        return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
    }

    vk::Extent2D chooseExtent(vk::Extent2D framebufferSize, const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        return {
            std::clamp<uint32_t>(framebufferSize.width, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(framebufferSize.height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)
        };
    }

    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        bool supportsMailbox = std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value) {
            return vk::PresentModeKHR::eMailbox == value;
        });

        return supportsMailbox ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
    }

    vk::Format findSupportedFormat(const vk::raii::PhysicalDevice& physicalDevice,
                                   const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                   vk::FormatFeatureFlags features) {
        for (const auto format : candidates) {
            vk::FormatProperties properties = physicalDevice.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features) {
                return format;
            }
            if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format");
    }

    vk::Format findDepthFormat(const vk::raii::PhysicalDevice& physicalDevice) {
        return findSupportedFormat(physicalDevice,
                                   {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                                   vk::ImageTiling::eOptimal,
                                   vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }
}

SwapChain::SwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device) {
    std::vector<vk::SurfaceFormatKHR> formats = device.getPhysicalDevice().getSurfaceFormatsKHR(surface);
    surfaceFormat_ = chooseSurfaceFormat(formats);

    createSwapChain(surface, framebufferSize, device);
    createColorResources(device);
    createDepthResources(device);
}

void SwapChain::createSwapChain(vk::SurfaceKHR surface, vk::Extent2D framebufferSize, const VulkanDevice& device) {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(
        surface);
    extent_ = chooseExtent(framebufferSize, surfaceCapabilities);

    std::vector<vk::PresentModeKHR> availablePresentModes =
            device.getPhysicalDevice().getSurfacePresentModesKHR(surface);

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = surface,
        .minImageCount = chooseMinImageCount(surfaceCapabilities),
        .imageFormat = surfaceFormat_.format,
        .imageColorSpace = surfaceFormat_.colorSpace,
        .imageExtent = extent_,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = choosePresentMode(availablePresentModes),
        .clipped = true
    };

    swapChain_ = vk::raii::SwapchainKHR(device.getDevice(), createInfo);

    std::vector<vk::Image> rawImages = swapChain_.getImages();
    images_.reserve(rawImages.size());
    for (const auto& image : rawImages) {
        images_.push_back({
            .image = image,
            .view = Image::createView(device, image, surfaceFormat_.format, vk::ImageAspectFlagBits::eColor)
        });
    }
}

void SwapChain::createColorResources(const VulkanDevice& device) {
    ImageConfig config {
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransientAttachment  | vk::ImageUsageFlagBits::eColorAttachment,
        .memory = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .aspect = vk::ImageAspectFlagBits::eColor
    };

    colorImage_ = Image::createResource(device, extent_.width, extent_.height, 1, config, surfaceFormat_.format, device.getMsaaSamples());
}

void SwapChain::createDepthResources(const VulkanDevice& device) {
    depthFormat_ = findDepthFormat(device.getPhysicalDevice());

    ImageConfig config {
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
        .memory = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .aspect = vk::ImageAspectFlagBits::eDepth
    };

    depthImage_ = Image::createResource(device, extent_.width, extent_.height, 1, config, depthFormat_, device.getMsaaSamples());
}

void SwapChain::recreate(const Window& window, const VulkanDevice& device) {
    auto size = window.getFramebufferSize();
    while (size.width == 0 || size.height == 0) {
        size = window.getFramebufferSize();
        window.waitEvents();
    }

    device.getDevice().waitIdle();

    images_.clear();
    swapChain_ = nullptr;

    createSwapChain(window.getSurface(), window.getFramebufferSize(), device);
    createColorResources(device);
    createDepthResources(device);
}
