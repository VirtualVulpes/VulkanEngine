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

    void loadResources();

    void populateScene();

    void update(float deltaTime);

    float calculateDeltaTime();

    void drawFrame();

    GLFWManager glfwManager_;
    vk::raii::Context context_;
    vk::raii::Instance instance_{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger_{nullptr};
    Window window_;
    InputManager& input_;
    VulkanDevice device_;
    SwapChain swapChain_;
    ResourceManager resources_;

    std::vector<FrameResources> frameResources_;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores_; // outside of frame resources, not one per frame
    uint32_t frameIndex_{0};

    Scene scene_;

    std::chrono::steady_clock::time_point lastTime_;
};
