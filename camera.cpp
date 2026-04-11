#include "camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "constants.h"
#include "input_manager.h"

Camera::Camera(glm::vec3 position, float pitch, float yaw, float aspectRatio, float fov) : position_(position),
    yaw_(yaw), pitch_(pitch), aspectRatio_(aspectRatio), fov_(fov) {
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(position_, position_ + forward_, Config::WorldUp);
}

glm::mat4 Camera::getProjection() const {
    glm::mat4 proj = glm::perspective(fov_, aspectRatio_, zNear_, zFar_);
    proj[1][1] *= -1;
    return proj;
}

void Camera::update(float deltaTime, const InputManager& input) {
    if (input.isKeyPressed(Key::W)) position_ += forward_ * speed_ * deltaTime;
    if (input.isKeyPressed(Key::S)) position_ -= forward_ * speed_ * deltaTime;
    if (input.isKeyPressed(Key::A)) position_ -= right_ * speed_ * deltaTime;
    if (input.isKeyPressed(Key::D)) position_ += right_ * speed_ * deltaTime;
    if (input.isKeyPressed(Key::C)) position_ -= Config::WorldUp * speed_ * deltaTime;
    if (input.isKeyPressed(Key::Space)) position_ += Config::WorldUp * speed_ * deltaTime;

    input.isKeyPressed(Key::Shift) ? speed_ = 9.0f : speed_ = 3.0f;

    glm::vec2 delta = input.getMouseDelta();
    yaw_ += delta.x * sensitivity_;
    pitch_ -= delta.y * sensitivity_;
    pitch_ = glm::clamp(pitch_, -89.0f, 89.0f);

    forward_ = glm::normalize(glm::vec3{
        cos(glm::radians(yaw_)) * cos(glm::radians(pitch_)),
        sin(glm::radians(pitch_)),
        sin(glm::radians(yaw_)) * cos(glm::radians(pitch_))
    });
    right_ = glm::normalize(glm::cross(forward_, Config::WorldUp));
}
