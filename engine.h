#pragma once

#include <chrono>

#include "window.h"
#include "vulkan_device.h"
#include "frame_resources.h"
#include "glfw_manager.h"
#include "graphics_pipeline.h"
#include "mesh.h"
#include "swap_chain.h"
#include "object.h"

class Engine
{
public:
    Engine();
    void run();

private:
    vk::raii::Instance createInstance();
    vk::raii::DebugUtilsMessengerEXT createDebugMessenger();

    void createImageResources();
    void createFrameResources();

    vk::raii::DescriptorPool createDescriptorPool();
    vk::raii::DescriptorSetLayout createDescriptorSetLayout();

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
    vk::raii::DescriptorSetLayout frameDescriptorSetLayout{nullptr};
    vk::raii::DescriptorSetLayout objectDescriptorSetLayout{nullptr};
    GraphicsPipeline graphicsPipeline;

    vk::raii::DescriptorPool descriptorPool{nullptr};

    std::vector<FrameResources> frameResources;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    uint32_t frameIndex{0};

    std::vector<Mesh> meshes{};
    std::vector<Object> objects{};

    std::chrono::steady_clock::time_point lastTime;
};
