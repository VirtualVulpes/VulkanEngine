#pragma once

class Transform {
public:
    Transform(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f),
              glm::vec3 scale = glm::vec3(1.0f));

    const glm::mat4& getModelMatrix() const;

    bool getIsDirty() const { return isDirty; }

    void setPosition(glm::vec3 newPos);

    void setScale(glm::vec3 newScale);

    void rotate(glm::vec3 offset);

    void move(glm::vec3 offset);

private:
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    mutable glm::mat4 model{glm::mat4(1.0f)};

    mutable bool isDirty{true};

    void recalculateModelMatrix() const;
};
