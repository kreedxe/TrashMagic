#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Window;

class Camera
{
public:
    enum class ProjectionMode
    {
        Perspective,
        Orthographic
    };

    Camera(
        const glm::vec3& position,
        const glm::vec3& target,
        float verticalFieldOfViewDegrees = 45.0f,
        float nearPlane = 0.1f,
        float farPlane = 100.0f);
    ~Camera();

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix(float aspectRatio) const;
    void setProjectionMode(ProjectionMode mode);
    ProjectionMode projectionMode() const;
    void setFieldOfViewDegrees(float fieldOfViewDegrees);
    void setOrthographicHeight(float height);
    float orthographicHeight() const;
    void setClipPlanes(float nearPlane, float farPlane);
    float nearPlane() const;
    float farPlane() const;
    void update(const Window& window, float deltaSeconds, bool controlsEnabled = true);
    void setMovementSpeed(float speed);
    void setMouseSensitivity(float sensitivity);
    const glm::vec3& position() const;
    const glm::vec3& forward() const;
    float fieldOfViewDegrees() const;
    void destroy();
    bool isDestroyed() const;

private:
    glm::vec3 m_position;
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_forward;
    float m_yawDegrees = -90.0f;
    float m_pitchDegrees = 0.0f;
    float m_verticalFieldOfViewDegrees;
    float m_nearPlane;
    float m_farPlane;
    float m_orthographicHeight = 20.0f;
    ProjectionMode m_projectionMode = ProjectionMode::Perspective;
    float m_movementSpeed = 4.0f;
    float m_mouseSensitivity = 0.1f;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouseSample = true;
    bool m_destroyed = false;
};
