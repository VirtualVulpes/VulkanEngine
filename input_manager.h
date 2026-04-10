#pragma once

#include <bitset>
#include <GLFW/glfw3.h>

enum class Key : int {
    W = GLFW_KEY_W,
    S = GLFW_KEY_S,
    A = GLFW_KEY_A,
    D = GLFW_KEY_D,
    C = GLFW_KEY_C,
    SHIFT = GLFW_KEY_LEFT_SHIFT,
    SPACE = GLFW_KEY_SPACE,
    ESCAPE = GLFW_KEY_ESCAPE,
    F11 = GLFW_KEY_F11
};

struct InputState {
    std::bitset<GLFW_KEY_LAST> keys{};
    std::bitset<5> mouseButtons{};
    glm::vec2 mousePosition{};
};

class InputManager {
public:
    InputManager(GLFWwindow* window);

    void update();

    InputState& getCurrentState() { return currentState_; };
    const InputState& getCurrentState() const { return currentState_; };

    InputState& getPreviousState() { return previousState_; };
    const InputState& getPreviousState() const { return previousState_; };

    bool isKeyPressed(Key key) const { return currentState_.keys.test(static_cast<int>(key)); }

    bool isKeyJustPressed(Key key) const {
        return currentState_.keys.test(static_cast<int>(key)) && !previousState_.keys.test(static_cast<int>(key));
    }

    glm::vec2 getMouseDelta() const { return currentState_.mousePosition - previousState_.mousePosition; };

private:
    InputState currentState_{};
    InputState previousState_{};
};
