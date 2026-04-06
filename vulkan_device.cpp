#include "vulkan_device.h"

#include "constants.h"

VulkanDevice::VulkanDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface)
{
    findPhysicalDevice(instance, surface);
    createLogicalDevice();
    createCommandPool();
}

void VulkanDevice::findPhysicalDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface)
{
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

    for (const auto& availableDevice : devices)
    {
        bool supportsVulkanVersion = availableDevice.getProperties().apiVersion >= vk::ApiVersion14;

        uint32_t foundQueueIndex = VK_QUEUE_FAMILY_IGNORED;

        auto queueFamilies = availableDevice.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); i++)
        {
            if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                availableDevice.getSurfaceSupportKHR(i, *surface))
            {
                foundQueueIndex = i;
                break;
            }
        }
        bool supportsQueue = foundQueueIndex != VK_QUEUE_FAMILY_IGNORED;

        auto availableExtensions = availableDevice.enumerateDeviceExtensionProperties();
        bool supportsExtensions = std::ranges::all_of(Config::requiredExtensions, [&](auto requiredExtension)
        {
            return std::ranges::any_of(availableExtensions, [&](const auto& extension)
            {
                return strcmp(extension.extensionName, requiredExtension) == 0;
            });
        });

        auto features = availableDevice.getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        >();

        bool supportsRequiredFeatures =
            features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
            features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        if (supportsVulkanVersion && supportsQueue && supportsExtensions && supportsRequiredFeatures)
        {
            physicalDevice = availableDevice;
            queueIndex = foundQueueIndex;
            return;
        }
    }

    throw std::runtime_error("no suitable device found");
}

void VulkanDevice::createLogicalDevice()
{
    vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        requiredFeatures = {
            {},
            {.shaderDrawParameters = true},
            {.synchronization2 = true, .dynamicRendering = true},
            {.extendedDynamicState = true}
        };

    // only one queue, so priority doesn't matter
    float queuePriority = 0.5f;
    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = queueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo createInfo{
        .pNext = &requiredFeatures.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(Config::requiredExtensions.size()),
        .ppEnabledExtensionNames = Config::requiredExtensions.data()
    };

    device = vk::raii::Device(physicalDevice, createInfo);
    graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
}

void VulkanDevice::createCommandPool()
{
    vk::CommandPoolCreateInfo poolCreateInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = queueIndex
    };

    commandPool = vk::raii::CommandPool{device, poolCreateInfo};
}