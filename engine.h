#pragma once

#include "window.h"
#include "vulkan_device.h"
#include "frame_resources.h"
#include "glfw_manager.h"
#include "graphics_pipeline.h"
#include "mesh.h"
#include "swap_chain.h"

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

    vk::raii::DescriptorPool createDescriptorPool() const;
    vk::raii::DescriptorSetLayout createDescriptorSetLayout();

    void drawFrame();

    GLFWManager glfwManager;
    vk::raii::Context context;
    vk::raii::Instance instance;
    vk::raii::DebugUtilsMessengerEXT debugMessenger;
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

    std::vector<Mesh> meshes;

    std::vector<vk::raii::DescriptorSet> descriptorSets;
};
