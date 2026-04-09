#pragma once

#include <chrono>

#include "window.h"
#include "vulkan_device.h"
#include "frame_resources.h"
#include "glfw_manager.h"
#include "graphics_pipeline.h"
#include "swap_chain.h"
#include "resource_manager.h"
#include "scene.h"

class Engine {
public:
    Engine();

    void run();

private:
    vk::raii::Instance createInstance();

    vk::raii::DebugUtilsMessengerEXT createDebugMessenger();

    void createImageResources();

    void createFrameResources();

    void loadAssets();

    void createScene();

    void update();

    void drawFrame();

    GLFWManager glfwManager;
    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
    Window window;
    VulkanDevice device;
    SwapChain swapChain;
    ResourceManager resources;
    GraphicsPipeline graphicsPipeline;

    std::vector<FrameResources> frameResources;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores; // outside of frame resources, not one per frame
    uint32_t frameIndex{0};

    Scene scene{};

    std::chrono::steady_clock::time_point lastTime;
};
