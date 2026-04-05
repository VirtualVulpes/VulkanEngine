#pragma once

#include <string_view>

#include <GLFW/glfw3.h>

class Window
{
public:
    Window(uint32_t width, uint32_t height, std::string_view title, const vk::raii::Instance& instance);
    const vk::raii::SurfaceKHR& getSurface() const { return surface; }
    vk::Extent2D getFramebufferSize() const;
    bool shouldClose() const;

    bool needsResize() const { return resizePending; }
    void onResize() { resizePending = true; }
    void clearResize() { resizePending = false; }

    void pollEvents() const;
    void waitEvents() const;
    ~Window();

private:
    GLFWwindow* window{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    bool resizePending{false};
};
