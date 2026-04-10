#pragma once

#include "constants.h"

class InputManager;
enum class InputAction;

class Camera {
public:
    Camera(glm::vec3 position = {0.0f, 2.0f, 2.0f},
           float yaw = 0.0f,
           float pitch = -90.0f,
           float aspectRatio = static_cast<float>(Config::WIDTH) / Config::HEIGHT,
           float fov = glm::radians(60.0f));

    glm::mat4 getView() const;

    glm::mat4 getProjection() const;

    void setAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }

    void update(float deltaTime, const InputManager& input);

private:
    glm::vec3 position_;
    float yaw_;
    float pitch_;
    float aspectRatio_;
    float fov_;

    glm::vec3 forward_{};
    glm::vec3 right_{};

    float speed_{3.0f};
    float sensitivity_{0.15f};

    float zNear_{0.1f};
    float zFar_{100.0f};
};
