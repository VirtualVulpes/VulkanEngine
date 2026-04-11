#include "glfw_input_translator.h"

#include <GLFW/glfw3.h>

Key glfw::toKey(int glfwKey) {
    switch (glfwKey) {
        case GLFW_KEY_W: return Key::W;
        case GLFW_KEY_S: return Key::S;
        case GLFW_KEY_A: return Key::A;
        case GLFW_KEY_D: return Key::D;
        case GLFW_KEY_C: return Key::C;
        case GLFW_KEY_LEFT_SHIFT: return Key::Shift;
        case GLFW_KEY_SPACE: return Key::Space;
        case GLFW_KEY_ESCAPE: return Key::Escape;
        case GLFW_KEY_F11: return Key::F11;
        default: return Key::Unknown;
    }
}

InputAction glfw::toAction(int glfwAction) {
    switch (glfwAction) {
        case GLFW_PRESS: return InputAction::Press;
        case GLFW_RELEASE: return InputAction::Release;
        default: return InputAction::Unknown;
    }
}

MouseButton glfw::toMouseButton(int glfwMouseButton) {
    switch (glfwMouseButton) {
        case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4: return MouseButton::Mouse4;
        case GLFW_MOUSE_BUTTON_5: return MouseButton::Mouse5;
        default: return MouseButton::Unknown;
    }
}
