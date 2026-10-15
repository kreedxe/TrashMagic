#include "Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::matrix(float elapsedSeconds) const
{
    glm::mat4 result(1.0f);
    result = glm::translate(result, position);

    const glm::vec3 currentRotation = rotation + angularVelocity * elapsedSeconds;
    result = glm::rotate(result, currentRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    result = glm::rotate(result, currentRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    result = glm::rotate(result, currentRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    return glm::scale(result, scale);
}
