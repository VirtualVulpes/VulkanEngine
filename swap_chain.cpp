#include "swap_chain.h"

#include "vulkan_device.h"
#include "window.h"

namespace
{
    uint32_t chooseMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities)
    {
        auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
        if ((surfaceCapabilities.maxImageCount > 0) && (surfaceCapabilities.maxImageCount < minImageCount))
        {
            minImageCount = surfaceCapabilities.maxImageCount;
        }
        return minImageCount;
    }

    vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        assert(!availableFormats.empty());

        const auto formatIt = std::ranges::find_if(availableFormats, [&](const auto& format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

        return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
    }

    vk::Extent2D chooseExtent(vk::Extent2D framebufferSize, const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        return {
            std::clamp<uint32_t>(framebufferSize.width, capabilities.minImageExtent.width,
                                 capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(framebufferSize.height, capabilities.minImageExtent.height,
                                 capabilities.maxImageExtent.height)
        };
    }

    vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        bool supportsMailbox = std::ranges::any_of(availablePresentModes, [](const vk::PresentModeKHR value)
        {
            return vk::PresentModeKHR::eMailbox == value;
        });

        return supportsMailbox ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
    }
}

SwapChain::SwapChain(const vk::raii::SurfaceKHR& surface, vk::Extent2D framebufferSize, const VulkanDevice& device)
{
    create(surface, framebufferSize, device);
    createImageViews(device);
}

void SwapChain::create(const vk::raii::SurfaceKHR& surface, vk::Extent2D framebufferSize, const VulkanDevice& device)
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(
        surface);
    extent = chooseExtent(framebufferSize, surfaceCapabilities);

    std::vector<vk::SurfaceFormatKHR> formats = device.getPhysicalDevice().getSurfaceFormatsKHR(surface);
    surfaceFormat = chooseSurfaceFormat(formats);

    std::vector<vk::PresentModeKHR> availablePresentModes = device.getPhysicalDevice().getSurfacePresentModesKHR(
        surface);

    vk::SwapchainCreateInfoKHR createInfo{
        .surface = surface,
        .minImageCount = chooseMinImageCount(surfaceCapabilities),
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = choosePresentMode(availablePresentModes),
        .clipped = true
    };

    swapChain = vk::raii::SwapchainKHR(device.getDevice(), createInfo);
    images = swapChain.getImages();
}

void SwapChain::createImageViews(const VulkanDevice& device)
{
    assert(imageViews.empty());

    vk::ImageViewCreateInfo createInfo{
        .viewType = vk::ImageViewType::e2D,
        .format = surfaceFormat.format,
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1}
    };

    for (const auto& image : images)
    {
        createInfo.image = image;
        imageViews.emplace_back(device.getDevice(), createInfo);
    }
}

void SwapChain::recreate(const Window& window, const VulkanDevice& device)
{
    auto size = window.getFramebufferSize();
    while (size.width == 0 || size.height == 0) {
        size = window.getFramebufferSize();
        window.waitEvents();
    }

    device.getDevice().waitIdle();

    imageViews.clear();
    swapChain = nullptr;

    create(window.getSurface(), window.getFramebufferSize(), device);
    createImageViews(device);
}
