#include "engine.h"

#include <iostream>

#include "constants.h"
#include "object.h"
#include "transform.h"

namespace {
    void checkLayers(const vk::raii::Context& context) {
        std::vector<vk::LayerProperties> availableLayers = context.enumerateInstanceLayerProperties();

        if (!std::ranges::all_of(Config::requiredLayers, [&](auto requiredLayer) {
            return std::ranges::any_of(availableLayers, [&](const auto& layer) {
                return strcmp(layer.layerName, requiredLayer) == 0;
            });
        })) {
            throw std::runtime_error("layers not supported");
        }
    }

    std::vector<const char *> checkExtensions(const vk::raii::Context& context) {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (Config::enableValidationLayers) {
            requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        std::vector availableExtensions = context.enumerateInstanceExtensionProperties();

        if (!std::ranges::all_of(requiredExtensions, [&](auto requiredExtension) {
            return std::ranges::any_of(availableExtensions, [&](const auto& extension) {
                return strcmp(extension.extensionName, requiredExtension) == 0;
            });
        })) {
            throw std::runtime_error("extensions not supported");
        }

        return requiredExtensions;
    }

    VkBool32 debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                           vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
                           const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: type " << to_string(messageTypes) << " msg: " << pCallbackData->pMessage <<
                std::endl;

        return vk::False;
    }
}

Engine::Engine() : glfwManager(),
                   instance(createInstance()),
                   debugMessenger(createDebugMessenger()),
                   window(Config::WIDTH, Config::HEIGHT, "Vulkan Engine", instance),
                   device(instance, window.getSurface()),
                   swapChain(window.getSurface(), window.getFramebufferSize(), device),
                   frameDescriptorSetLayout(createDescriptorSetLayout()),
                   objectDescriptorSetLayout(createDescriptorSetLayout()),
                   graphicsPipeline(device.getDevice(), swapChain.getColorFormat(), swapChain.getDepthFormat(),
                                    frameDescriptorSetLayout,
                                    objectDescriptorSetLayout),
                   descriptorPool(createDescriptorPool()) {
    createImageResources();
    createFrameResources();

    meshes.emplace_back(device, Shape::Plane);
    meshes.emplace_back(device, Shape::Cube);
    Transform transform{};
    objects.emplace_back(device, descriptorPool, objectDescriptorSetLayout, transform, meshes.at(0), 0);
    transform.setPosition({0.0f, 0.0f, 0.5f});
    transform.rotate({0.0f, 90.0f, -45.0f});
    transform.setScale({0.5f, 0.5f,  0.5f});
    objects.emplace_back(device, descriptorPool, objectDescriptorSetLayout, transform, meshes.at(1), 1);
}

void Engine::run() {
    lastTime = std::chrono::steady_clock::now();

    while (!window.shouldClose()) {
        window.pollEvents();
        update();
        drawFrame();
    }

    device.getDevice().waitIdle();
}

vk::raii::Instance Engine::createInstance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Vulkan Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Vulkan Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    checkLayers(context);
    std::vector<const char *> extensions = checkExtensions(context);

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(Config::requiredLayers.size()),
        .ppEnabledLayerNames = Config::requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    return {context, createInfo};
}

vk::raii::DebugUtilsMessengerEXT Engine::createDebugMessenger() {
    if constexpr (!Config::enableValidationLayers) return nullptr;

    constexpr vk::DebugUtilsMessageSeverityFlagsEXT severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    constexpr vk::DebugUtilsMessageTypeFlagsEXT type = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        .messageSeverity = severity,
        .messageType = type,
        .pfnUserCallback = debugCallback
    };

    return {instance, createInfo};
}

void Engine::createImageResources() {
    assert(renderFinishedSemaphores.empty());

    renderFinishedSemaphores.reserve(swapChain.getImageSize());

    for (size_t i = 0; i < swapChain.getImageSize(); i++) {
        renderFinishedSemaphores.emplace_back(device.getDevice(), vk::SemaphoreCreateInfo());
    }
}

void Engine::createFrameResources() {
    assert(frameResources.empty());

    frameResources.reserve(Config::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; ++i) {
        frameResources.emplace_back(device, descriptorPool, frameDescriptorSetLayout, swapChain.getTextureImages());
    }
}

vk::raii::DescriptorPool Engine::createDescriptorPool() {
    std::array poolSize{
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = Config::MAX_FRAMES_IN_FLIGHT * Config::MAX_OBJECTS
        },
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = Config::MAX_FRAMES_IN_FLIGHT * Config::MAX_TEXTURES
        }
    };

    vk::DescriptorPoolCreateInfo createInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = Config::MAX_FRAMES_IN_FLIGHT * (Config::MAX_OBJECTS + Config::MAX_TEXTURES),
        .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
        .pPoolSizes = poolSize.data()
    };

    return {device.getDevice(), createInfo};
}

vk::raii::DescriptorSetLayout Engine::createDescriptorSetLayout() {
    std::array bindings{
        vk::DescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
        },
        vk::DescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = static_cast<uint32_t>(swapChain.getTextureImages().size()),
            .stageFlags = vk::ShaderStageFlagBits::eFragment
        },
    };
    vk::DescriptorSetLayoutCreateInfo frameLayoutInfo{.bindingCount = bindings.size(), .pBindings = bindings.data()};

    return {device.getDevice(), frameLayoutInfo};
}

void Engine::update() {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;

    objects.at(0).getTransform().rotate(glm::vec3(0.0f, 0.0f, 90.0f) * deltaTime);
    objects.at(1).getTransform().rotate(glm::vec3(-40.0f, 30.0f, -20.0f) * deltaTime);
}

void Engine::drawFrame() {
    FrameResources& currentFrame = frameResources[frameIndex];
    currentFrame.beginFrame(device.getDevice());

    auto [result, imageIndex] = swapChain.getSwapChain().acquireNextImage(
        UINT64_MAX, *currentFrame.getPresentCompleteSemaphore(), nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        swapChain.recreate(window, device);
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    FrameData frameData{
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),
                                 static_cast<float>(swapChain.getExtent().width) /
                                 static_cast<float>(swapChain.getExtent().height),
                                 0.1f, 10.0f)
    };
    frameData.proj[1][1] *= -1;
    currentFrame.updateFrameData(frameData);

    for (const auto& object : objects) {
        object.updateTransform();
    }

    device.getDevice().resetFences(*currentFrame.getInFlightFence());

    RenderTarget target{
        .colorImage = swapChain.getImage(imageIndex),
        .colorImageView = swapChain.getImageView(imageIndex),
        .depthImage = swapChain.getDepthImage(),
        .depthImageView = swapChain.getDepthImageView(),
        .extent = swapChain.getExtent()
    };

    currentFrame.recordCommandBuffer(*graphicsPipeline.getPipeline(), *graphicsPipeline.getLayout(),
                                     *currentFrame.getDescriptorSet(), target, frameIndex, objects);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*currentFrame.getPresentCompleteSemaphore(),
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*currentFrame.getCommandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores[imageIndex]
    };
    device.getQueue().submit(submitInfo, *currentFrame.getInFlightFence());

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapChain.getSwapChain(),
        .pImageIndices = &imageIndex
    };

    result = device.getQueue().presentKHR(presentInfoKHR);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || window.needsResize()) {
        window.clearResize();
        swapChain.recreate(window, device);
    } else {
        assert(result == vk::Result::eSuccess);
    }

    frameIndex = (frameIndex + 1) % Config::MAX_FRAMES_IN_FLIGHT;
}
