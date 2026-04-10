#pragma once

#include <string_view>

#include <GLFW/glfw3.h>

#include "constants.h"
#include "input_manager.h"

class Window {
public:
    Window(glm::ivec2 windowPos, glm::ivec2 windowSize, std::string_view title, const vk::raii::Instance& instance);

    GLFWwindow* createHandle(std::string_view title);

    const vk::raii::SurfaceKHR& getSurface() const { return surface_; }

    vk::Extent2D getFramebufferSize() const;

    InputManager& getInputManager() { return input_; }
    const InputManager& getInputManager() const { return input_; }

    bool shouldClose() const;

    bool needsResize() const { return resizePending_; }
    void onResize() { resizePending_ = true; }
    void clearResize() { resizePending_ = false; }

    void update();

    void waitEvents() const;

    void toggleFullscreen();

    void setWindowed();

    void setBorderlessFullscreen();

    void setCursorCaptured(bool captured);

    void close() { glfwSetWindowShouldClose(window_, true); }
    GLFWwindow* getRawWindow() { return window_; }

    ~Window();

private:
    glm::ivec2 windowPos_;
    glm::ivec2 windowSize_;

    GLFWwindow* window_;
    vk::raii::SurfaceKHR surface_{nullptr};
    bool resizePending_{false};
    bool isFullscreen_{false};

    InputManager input_;
};
