#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Transform
{
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 angularVelocity{0.0f};

    glm::mat4 matrix(float elapsedSeconds = 0.0f) const;
};
