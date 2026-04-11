#include "input_manager.h"

InputManager::InputManager(glm::dvec2 mousePos) {
    currentState_.mousePosition = mousePos;
}

void InputManager::update() {
    previousState_ = currentState_;
}

void InputManager::lateUpdate() {
    if (Clock::now() < ignoreMouseExpiry_) {
        previousState_.mousePosition = currentState_.mousePosition;
    }
}

// does not clear mouse position
void InputManager::reset() {
    currentState_.keys.reset();
    currentState_.mouseButtons.reset();
    previousState_.keys.reset();
    previousState_.mouseButtons.reset();
}

void InputManager::updateMousePosition(glm::dvec2 newPosition) {
    currentState_.mousePosition = newPosition;
    previousState_.mousePosition = newPosition;
}

void InputManager::onKeyEvent(Key key, InputAction action) {
    if (key == Key::Unknown || key == Key::Count) return;

    if (action == InputAction::Press) currentState_.keys.set(static_cast<size_t>(key));
    else if (action == InputAction::Release) currentState_.keys.reset(static_cast<size_t>(key));
}

void InputManager::onMouseButton(MouseButton button, InputAction action) {
    if (action == InputAction::Press) currentState_.mouseButtons.set(static_cast<size_t>(button));
    else if (action == InputAction::Release) currentState_.mouseButtons.reset(static_cast<size_t>(button));
}

void InputManager::onMouseMove(glm::dvec2 mousePos) {
    currentState_.mousePosition = mousePos;
}

