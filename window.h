#pragma once

#include <GLFW/glfw3.h>

#include "input_manager.h"

class Window {
public:
    Window(glm::ivec2 windowSize, const std::string& title, const vk::raii::Instance& instance, bool fullscreen);

    const vk::raii::SurfaceKHR& getSurface() const { return surface_; }

    vk::Extent2D getFramebufferSize() const;

    InputManager& getInputManager() { return input_; }
    const InputManager& getInputManager() const { return input_; }

    bool shouldClose() const;

    bool needsResize() const { return resizePending_; }
    void clearResize() { resizePending_ = false; }

    void update();

    void waitEvents() const;

    void toggleFullscreen();

    void close() { glfwSetWindowShouldClose(window_, true); }

    bool isFocused() { return isFocused_; }

    void setFocused();

    glm::dvec2 getMousePosition() const;

    ~Window();

private:
    GLFWwindow* createHandle(const std::string& title);

    void setWindowed();

    void setBorderlessFullscreen();

    GLFWmonitor* getActiveMonitor() const;
    glm::ivec2 getDefaultPosition() const;

    void updateWindowLayout();
    void registerCallbacks();

    glm::ivec2 windowSize_;
    glm::ivec2 windowPos_;
    bool isFullscreen_{false};

    GLFWwindow* window_;
    vk::raii::SurfaceKHR surface_{nullptr};
    bool resizePending_{false};
    bool isFocused_{true};

    InputManager input_;
};
