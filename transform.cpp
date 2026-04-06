#include "transform.h"

#include <glm/ext/matrix_transform.hpp>

const glm::mat4& Transform::getModelMatrix() const {
    if (isDirty) recalculateModelMatrix();

    return model;
}

void Transform::setPosition(glm::vec3 newPos) {
    position = newPos;
    isDirty = true;
}

void Transform::setScale(glm::vec3 newScale) {
    scale = newScale;
    isDirty = true;
}

void Transform::rotate(glm::vec3 offset) {
    rotation += radians(offset);
    isDirty = true;
}

void Transform::recalculateModelMatrix() const {
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
    model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    isDirty = false;
}
