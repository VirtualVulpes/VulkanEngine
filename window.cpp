#include "window.h"

#include "glfw_input_translator.h"

Window::Window(glm::ivec2 windowPos, glm::ivec2 windowSize, const std::string& title,
               const vk::raii::Instance& instance) : windowPos_(windowPos), windowSize_(windowSize),
                                                     window_(createHandle(title)), input_(getMousePosition()) {
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window_, nullptr, &rawSurface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    surface_ = vk::raii::SurfaceKHR(instance, rawSurface);
    registerCallbacks();
}

GLFWwindow* Window::createHandle(const std::string& title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(windowSize_.x, windowSize_.y, title.c_str(), nullptr,
                                          nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* rawWindow, int width, int height) {
        auto* pWindow = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow));
        pWindow->resizePending_ = true;
    });

    glfwSetWindowFocusCallback(window, [](GLFWwindow* rawWindow, int focused) {
        auto* pWindow = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow));
        if (!focused) {
            pWindow->isFocused_ = false;
            glfwSetInputMode(pWindow->getRawWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            pWindow->input_.reset();
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

    input_.setIgnoreMouseDuration(100);
}

void Window::setWindowed() {
    isFullscreen_ = false;

    glfwSetWindowAttrib(window_, GLFW_DECORATED, GLFW_TRUE);

    glfwSetWindowPos(window_, windowPos_.x, windowPos_.y);
    glfwSetWindowSize(window_, windowSize_.x, windowSize_.y);
}

void Window::setBorderlessFullscreen() {
    updateWindowLayout();
    GLFWmonitor* monitor = getActiveMonitor();
    if (!monitor) return;

    isFullscreen_ = true;

    glfwSetWindowAttrib(window_, GLFW_DECORATED, GLFW_FALSE);

    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowPos(window_, mx, my);
    glfwSetWindowSize(window_, mode->width, mode->height);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::setFocused() {
    isFocused_ = true;

    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    input_.updateMousePosition(getMousePosition());
    input_.setIgnoreMouseDuration(200);
}

glm::dvec2 Window::getMousePosition() const {
    glm::dvec2 mousePos{};
    glfwGetCursorPos(window_, &mousePos.x, &mousePos.y);
    return mousePos;
}

Window::~Window() {
    glfwDestroyWindow(window_);
}

GLFWmonitor* Window::getActiveMonitor() const {
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);

    glm::ivec2 center{windowPos_.x + windowSize_.x / 2, windowPos_.y + windowSize_.y / 2};

    for (int i = 0; i < count; i++) {
        int xPos, yPos;
        glfwGetMonitorPos(monitors[i], &xPos, &yPos);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);

        // choose monitor that contains the center of the window
        if (xPos <= center.x && center.x < xPos + mode->width &&
            yPos <= center.y && center.y < yPos + mode->height) {
            return monitors[i];
        }
    }

    return glfwGetPrimaryMonitor();
}

void Window::updateWindowLayout() {
    glfwGetWindowSize(window_, &windowSize_.x, &windowSize_.y);
    glfwGetWindowPos(window_, &windowPos_.x, &windowPos_.y);
}

void Window::registerCallbacks() {
    glfwSetKeyCallback(window_, [](GLFWwindow* rawWindow, int key, int scancode, int action, int mods) {
        auto* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
        if (!pWindow->isFocused()) return;

        pWindow->getInputManager().onKeyEvent(glfw::toKey(key), glfw::toAction(action));
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* rawWindow, int button, int action, int mods) {
        auto* pWindow = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow));

        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1 && !pWindow->isFocused()) {
            pWindow->setFocused();
        }

        if (!pWindow->isFocused()) return;

        pWindow->getInputManager().onMouseButton(glfw::toMouseButton(button), glfw::toAction(action));
    });

    glfwSetCursorPosCallback(window_, [](GLFWwindow* rawWindow, double xpos, double ypos) {
        auto* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(rawWindow));
        if (!pWindow->isFocused()) return;

        pWindow->getInputManager().onMouseMove(glm::dvec2{xpos, ypos});
    });
}
