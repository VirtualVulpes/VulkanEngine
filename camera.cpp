#include "camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "constants.h"

Camera::Camera(glm::vec3 position, glm::vec3 viewDirection, float aspectRatio, float fov) : position_(position),
    viewDirection_(viewDirection), aspectRatio_(aspectRatio), fov_(fov) {
}

glm::mat4 Camera::getView() const {
    return glm::lookAt(position_, position_ + viewDirection_, Config::UP_DIRECTION);
}

glm::mat4 Camera::getProjection() const {
    glm::mat4 proj = glm::perspective(fov_, aspectRatio_, zNear_, zFar_);
    proj[1][1] *= -1;
    return proj;
}
