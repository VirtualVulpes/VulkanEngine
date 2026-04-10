#include "window.h"

namespace
{
    void framebufferResizeCallback(GLFWwindow* rawWindow, int width, int height) {
        auto window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
        window->onResize();
    }
}

Window::Window(uint32_t width, uint32_t height, std::string_view title, const vk::raii::Instance& instance)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.data(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);

    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window_, nullptr, &rawSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
    surface_ = vk::raii::SurfaceKHR(instance, rawSurface);
}

vk::Extent2D Window::getFramebufferSize() const
{
    int rawWidth, rawHeight;
    glfwGetFramebufferSize(window_, &rawWidth, &rawHeight);
    return {static_cast<uint32_t>(rawWidth), static_cast<uint32_t>(rawHeight)};
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

void Window::waitEvents() const
{
    glfwWaitEvents();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window_);
}

Window::~Window()
{
    glfwDestroyWindow(window_);
}
