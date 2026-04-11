#pragma once

#include <bitset>

#include "constants.h"

enum class InputAction {
    Press,
    Release,
    Unknown
};

enum class Key : int {
    W,
    S,
    A,
    D,
    C,
    Shift,
    Space,
    Escape,
    F11,
    Unknown,
    Count
};

enum class MouseButton : int {
    Left,
    Right,
    Middle,
    Mouse4,
    Mouse5,
    Unknown,
    Count
};

struct InputState {
    std::bitset<static_cast<size_t>(Key::Count)> keys{};
    std::bitset<static_cast<size_t>(MouseButton::Count)> mouseButtons{};
    glm::dvec2 mousePosition{};
};

class InputManager {
public:
    InputManager(glm::dvec2 mousePos);

    void update(); // runs before pollEvents()
    void lateUpdate(); // runs after pollEvents()

    InputState& getCurrentState() { return currentState_; };
    const InputState& getCurrentState() const { return currentState_; };

    InputState& getPreviousState() { return previousState_; };
    const InputState& getPreviousState() const { return previousState_; };

    bool isKeyPressed(Key key) const { return currentState_.keys.test(static_cast<size_t>(key)); }

    bool isKeyJustPressed(Key key) const {
        return currentState_.keys.test(static_cast<size_t>(key)) &&
               !previousState_.keys.test(static_cast<size_t>(key));
    }

    bool isMouseButtonPressed(MouseButton button) const {
        return currentState_.mouseButtons.test(static_cast<size_t>(button));
    }

    bool isMouseButtonJustPressed(MouseButton button) const {
        return currentState_.mouseButtons.test(static_cast<size_t>(button)) &&
               !previousState_.mouseButtons.test(static_cast<size_t>(button));
    }

    glm::vec2 getMouseDelta() const { return currentState_.mousePosition - previousState_.mousePosition; }

    void updateMousePosition(glm::dvec2 newPosition);

    void setIgnoreMouseDuration(int milliseconds) {
        ignoreMouseExpiry_ = Clock::now() + std::chrono::milliseconds(milliseconds);
    };

    void reset();

    void onKeyEvent(Key key, InputAction action);

    void onMouseButton(MouseButton button, InputAction action);

    void onMouseMove(glm::dvec2 mousePos);

private:
    InputState currentState_{};
    InputState previousState_{};

    std::chrono::steady_clock::time_point ignoreMouseExpiry_{};
};
