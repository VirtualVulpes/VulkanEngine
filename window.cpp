#include "window.h"

Window::Window(glm::ivec2 windowPos, glm::ivec2 windowSize, std::string_view title,
               const vk::raii::Instance& instance) : windowPos_(windowPos), windowSize_(windowSize), window_(createHandle(title)), input_(window_) {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window_, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    surface_ = vk::raii::SurfaceKHR(instance, rawSurface);
}

GLFWwindow* Window::createHandle(std::string_view title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(windowSize_.r, windowSize_.g, title.data(), nullptr,
                                          nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* rawWindow, int width, int height) {
        auto* window = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow));
        window->onResize();
    });

    glfwSetWindowFocusCallback(window, [](GLFWwindow* rawWindow, int focused) {
        auto* window = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
        if (!focused) {
            glfwSetInputMode(rawWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    });

    return window;
}

vk::Extent2D Window::getFramebufferSize() const {
    int rawWidth, rawHeight;
    glfwGetFramebufferSize(window_, &rawWidth, &rawHeight);
    return {static_cast<uint32_t>(rawWidth), static_cast<uint32_t>(rawHeight)};
}

void Window::update() {
    input_.update();
    glfwPollEvents();
}

void Window::waitEvents() const {
    glfwWaitEvents();
}

void Window::toggleFullscreen() {
    if (isFullscreen_) {
        setWindowed();
    } else {
        setBorderlessFullscreen();
    }
}

void Window::setWindowed() {
    isFullscreen_ = false;

    glfwSetWindowAttrib(window_, GLFW_DECORATED, GLFW_TRUE);

    glfwSetWindowPos(window_, windowPos_.x, windowPos_.y);
    glfwSetWindowSize(window_, windowSize_.x, windowSize_.y);
}

void Window::setBorderlessFullscreen() {
    isFullscreen_ = true;

    glfwGetWindowSize(window_, &windowSize_.x, &windowSize_.y);
    glfwGetWindowPos(window_, &windowPos_.x, &windowPos_.y);

    glfwSetWindowAttrib(window_, GLFW_DECORATED, GLFW_FALSE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    glfwSetWindowPos(window_, mx, my);
    glfwSetWindowSize(window_, mode->width, mode->height);
}

void Window::setCursorCaptured(bool captured) {
    glfwSetInputMode(window_, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

Window::~Window() {
    glfwDestroyWindow(window_);
}
