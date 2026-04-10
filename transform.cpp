#include "transform.h"

#include <glm/ext/matrix_transform.hpp>

Transform::Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) : position_(position),
    rotation_(glm::radians(rotation)), scale_(scale) {
}

const glm::mat4& Transform::getModelMatrix() const {
    if (isDirty_) recalculateModelMatrix();

    return model_;
}

void Transform::setPosition(glm::vec3 newPos) {
    position_ = newPos;
    isDirty_ = true;
}

void Transform::setScale(glm::vec3 newScale) {
    scale_ = newScale;
    isDirty_ = true;
}

void Transform::rotate(glm::vec3 offset) {
    rotation_ += radians(offset);
    isDirty_ = true;
}

void Transform::move(glm::vec3 offset) {
    position_ += offset;
    isDirty_ = true;
}

void Transform::recalculateModelMatrix() const {
    model_ = glm::mat4(1.0f);
    model_ = glm::translate(model_, position_);
    model_ = glm::rotate(model_, rotation_.y, glm::vec3(0, 1, 0));
    model_ = glm::rotate(model_, rotation_.x, glm::vec3(1, 0, 0));
    model_ = glm::rotate(model_, rotation_.z, glm::vec3(0, 0, 1));
    model_ = glm::scale(model_, scale_);

    isDirty_ = false;
}
