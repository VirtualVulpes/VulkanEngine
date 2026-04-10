#pragma once

#include <string_view>

#include <GLFW/glfw3.h>

class Window
{
public:
    Window(uint32_t width, uint32_t height, std::string_view title, const vk::raii::Instance& instance);
    const vk::raii::SurfaceKHR& getSurface() const { return surface_; }
    vk::Extent2D getFramebufferSize() const;
    bool shouldClose() const;

    bool needsResize() const { return resizePending_; }
    void onResize() { resizePending_ = true; }
    void clearResize() { resizePending_ = false; }

    void pollEvents() const;
    void waitEvents() const;
    ~Window();

private:
    GLFWwindow* window_{nullptr};
    vk::raii::SurfaceKHR surface_{nullptr};
    bool resizePending_{false};
};
