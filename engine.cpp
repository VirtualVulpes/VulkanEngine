#include "engine.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "constants.h"
#include "object.h"
#include "resource_manager.h"
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

Engine::Engine() : glfwManager_(),
                   instance_(createInstance()),
                   debugMessenger_(createDebugMessenger()),
                   window_(Config::WIDTH, Config::HEIGHT, "Vulkan Engine", instance_),
                   device_(instance_, window_.getSurface()),
                   swapChain_(window_.getSurface(), window_.getFramebufferSize(), device_),
                   resources_(device_),
                   graphicsPipeline_(device_, swapChain_.getColorFormat(), swapChain_.getDepthFormat(),
                                    resources_.getFrameLayout(), resources_.getMaterialLayout(),
                                    resources_.getObjectLayout()),
                   scene_(device_, resources_) {
    createImageResources();
    createFrameResources();
}

void Engine::loadAssets() {
    // --- Meshes ---

    resources_.loadMesh("plane", Shape::plane());
    resources_.loadMesh("cube", Shape::cube());
    resources_.loadMesh("viking_room.glb");

    // --- Textures ---

    Assets::SamplerConfig lowResSampler{
        .filter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eNearest,
        .addressMode = vk::SamplerAddressMode::eClampToEdge,
        .anisotropy = false
    };
    resources_.loadTexture("viking_room.ktx2");
    resources_.loadTexture("dirt.ktx2", lowResSampler);
}

void Engine::populateScene() {
    scene_.createObject(Transform{}, "viking_room", "viking_room");

    Transform cubeTransform{{-2.0f, 0.0f, 0.5f}, {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}};
    scene_.createObject(cubeTransform, "cube", "dirt");
}

void Engine::run() {
    loadAssets();
    populateScene();

    lastTime_ = std::chrono::steady_clock::now();
    while (!window_.shouldClose()) {
        window_.pollEvents();
        update();
        drawFrame();
    }

    device_.getDevice().waitIdle();
}

vk::raii::Instance Engine::createInstance() {
    constexpr vk::ApplicationInfo appInfo{
        .pApplicationName = "Vulkan Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Vulkan Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    checkLayers(context_);
    std::vector<const char *> extensions = checkExtensions(context_);

    vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(Config::requiredLayers.size()),
        .ppEnabledLayerNames = Config::requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    return {context_, createInfo};
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

    return {instance_, createInfo};
}

void Engine::createImageResources() {
    assert(renderFinishedSemaphores_.empty());

    renderFinishedSemaphores_.reserve(swapChain_.getImageSize());

    for (size_t i = 0; i < swapChain_.getImageSize(); i++) {
        renderFinishedSemaphores_.emplace_back(device_.getDevice(), vk::SemaphoreCreateInfo());
    }
}

void Engine::createFrameResources() {
    assert(frameResources_.empty());
    frameResources_.reserve(Config::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < Config::MAX_FRAMES_IN_FLIGHT; ++i) {
        frameResources_.emplace_back(device_, resources_.getPool(), resources_.getFrameLayout());
    }
}

void Engine::update() {
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime_).count();
    lastTime_ = currentTime;

    for (auto& [id, object] : scene_.getObjects()) {
        if (id == 0) {
            object.getTransform().rotate(glm::vec3(0.0f, 45.0f, 0.0f) * deltaTime);
        }

        if (id == 1) {
            object.getTransform().rotate(glm::vec3(-40.0f, 30.0f, -20.0f) * deltaTime);
        }
    }
}

void Engine::drawFrame() {
    FrameResources& currentFrame = frameResources_[frameIndex_];
    currentFrame.beginFrame(device_.getDevice());

    auto [result, imageIndex] = swapChain_.getSwapChain().acquireNextImage(
        UINT64_MAX, *currentFrame.getPresentCompleteSemaphore(), nullptr);

    if (result == vk::Result::eErrorOutOfDateKHR) {
        swapChain_.recreate(window_, device_);
        scene_.getActiveCamera().setAspectRatio(static_cast<float>(swapChain_.getExtent().width) / swapChain_.getExtent().height);
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    FrameData frameData{
        .view = scene_.getActiveCamera().getView(),
        .proj = scene_.getActiveCamera().getProjection()
    };
    currentFrame.updateFrameData(frameData);

    std::vector<const Object *> drawList;
    drawList.reserve(scene_.getObjects().size());
    for (ObjectID id : scene_.getDrawOrder()) {
        scene_.getObject(id).updateTransform();
        drawList.push_back(&scene_.getObject(id));
    }

    device_.getDevice().resetFences(*currentFrame.getInFlightFence());

    RenderTarget target{
        .swapChainImage = swapChain_.getImage(imageIndex),
        .swapChainImageView = swapChain_.getImageView(imageIndex),
        .colorImage = swapChain_.getColorImage(),
        .colorImageView = swapChain_.getColorImageView(),
        .depthImage = swapChain_.getDepthImage(),
        .depthImageView = swapChain_.getDepthImageView(),
        .extent = swapChain_.getExtent()
    };

    currentFrame.recordCommandBuffer(*graphicsPipeline_.getPipeline(), *graphicsPipeline_.getLayout(),
                                     *currentFrame.getDescriptorSet(), target, frameIndex_, drawList);

    vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*currentFrame.getPresentCompleteSemaphore(),
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*currentFrame.getCommandBuffer(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*renderFinishedSemaphores_[imageIndex]
    };
    device_.getQueue().submit(submitInfo, *currentFrame.getInFlightFence());

    const vk::PresentInfoKHR presentInfoKHR{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*renderFinishedSemaphores_[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &*swapChain_.getSwapChain(),
        .pImageIndices = &imageIndex
    };

    result = device_.getQueue().presentKHR(presentInfoKHR);
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || window_.needsResize()) {
        window_.clearResize();
        swapChain_.recreate(window_, device_);
        scene_.getActiveCamera().setAspectRatio(static_cast<float>(swapChain_.getExtent().width) / swapChain_.getExtent().height);
    } else {
        assert(result == vk::Result::eSuccess);
    }

    frameIndex_ = (frameIndex_ + 1) % Config::MAX_FRAMES_IN_FLIGHT;
}
