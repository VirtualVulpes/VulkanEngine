#pragma once
#include "constants.h"

class Camera {
public:
    Camera(glm::vec3 position = {0.0f, 2.0f, 2.0f}, glm::vec3 viewDirection = glm::normalize(glm::vec3{0.0f, -1.0f, -1.0f}),
           float aspectRatio = static_cast<float>(Config::WIDTH) / Config::HEIGHT, float fov = glm::radians(60.0f));

    glm::mat4 getView() const;

    glm::mat4 getProjection() const;

    void setAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }

private:
    glm::vec3 position_;
    glm::vec3 viewDirection_;
    float aspectRatio_;
    float fov_;

    float zNear_{0.1f};
    float zFar_{100.0f};
};
