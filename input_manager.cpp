#include "input_manager.h"

#include "window.h"

InputManager::InputManager(GLFWwindow* window) {
    glfwSetKeyCallback(window, [](GLFWwindow* rawWindow, int key, int scancode, int action, int mods) {
        if (key < 0 || key >= GLFW_KEY_LAST) return;
        auto& input = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow))->getInputManager();
        if (action == GLFW_PRESS) input.getCurrentState().keys.set(key);
        if (action == GLFW_RELEASE) input.getCurrentState().keys.reset(key);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* rawWindow, int button, int action, int mods) {
        auto* pWindow = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow));
        if (action == GLFW_PRESS) {
            if (button == GLFW_MOUSE_BUTTON_1) {
                glfwSetInputMode(pWindow->getRawWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                pWindow->getInputManager().getCurrentState().mouseButtons.set(button);
            }
        } else if (action == GLFW_RELEASE) {
            pWindow->getInputManager().getCurrentState().mouseButtons.reset(button);
        }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* rawWindow, double xpos, double ypos) {\
        auto& input = static_cast<Window *>(glfwGetWindowUserPointer(rawWindow))->getInputManager();
        input.getCurrentState().mousePosition = {xpos, ypos};
    });
}

void InputManager::update() {
    previousState_ = currentState_;
}