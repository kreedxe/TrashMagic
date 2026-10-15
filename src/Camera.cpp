#include "Camera.hpp"

#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

Camera::Camera(
    const glm::vec3& position,
    const glm::vec3& target,
    float verticalFieldOfViewDegrees,
    float nearPlane,
    float farPlane)
    : m_position(position),
      m_verticalFieldOfViewDegrees(verticalFieldOfViewDegrees),
      m_nearPlane(nearPlane),
      m_farPlane(farPlane)
{
    if (verticalFieldOfViewDegrees <= 0.0f || verticalFieldOfViewDegrees >= 180.0f
        || nearPlane <= 0.0f || farPlane <= nearPlane
        || glm::length(target - position) == 0.0f)
    {
        throw std::invalid_argument("Invalid camera position or projection parameters.");
    }

    m_forward = glm::normalize(target - position);
    m_yawDegrees = glm::degrees(std::atan2(m_forward.z, m_forward.x));
    m_pitchDegrees = glm::degrees(std::asin(m_forward.y));
}

Camera::~Camera()
{
    destroy();
}

glm::mat4 Camera::viewMatrix() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed camera.");
    }
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::projectionMatrix(float aspectRatio) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed camera.");
    }
    if (aspectRatio <= 0.0f)
    {
        throw std::invalid_argument("Camera aspect ratio must be greater than zero.");
    }
    return glm::perspective(
        glm::radians(m_verticalFieldOfViewDegrees),
        aspectRatio,
        m_nearPlane,
        m_farPlane);
}

void Camera::update(const Window& window, float deltaSeconds, bool controlsEnabled)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot update a destroyed camera.");
    }
    if (deltaSeconds < 0.0f)
    {
        throw std::invalid_argument("Camera delta time must be non-negative.");
    }

    if (controlsEnabled)
    {
        const float movement = m_movementSpeed * glm::min(deltaSeconds, 0.1f);
        const glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));
        glm::vec3 movementDirection(0.0f);
        if (window.isKeyDown(GLFW_KEY_W))
        {
            movementDirection += m_forward;
        }
        if (window.isKeyDown(GLFW_KEY_S))
        {
            movementDirection -= m_forward;
        }
        if (window.isKeyDown(GLFW_KEY_A))
        {
            movementDirection -= right;
        }
        if (window.isKeyDown(GLFW_KEY_D))
        {
            movementDirection += right;
        }
        if (glm::length(movementDirection) > 0.0f)
        {
            m_position += glm::normalize(movementDirection) * movement;
        }

        double mouseX = 0.0;
        double mouseY = 0.0;
        window.getCursorPosition(mouseX, mouseY);
        if (m_firstMouseSample)
        {
            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;
            m_firstMouseSample = false;
        }
        else
        {
            const float xOffset = static_cast<float>(mouseX - m_lastMouseX);
            const float yOffset = static_cast<float>(m_lastMouseY - mouseY);
            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;

            m_yawDegrees += xOffset * m_mouseSensitivity;
            m_pitchDegrees = glm::clamp(
                m_pitchDegrees + yOffset * m_mouseSensitivity,
                -89.0f,
                89.0f);
            const float yaw = glm::radians(m_yawDegrees);
            const float pitch = glm::radians(m_pitchDegrees);
            m_forward = glm::normalize(glm::vec3(
                std::cos(yaw) * std::cos(pitch),
                std::sin(pitch),
                std::sin(yaw) * std::cos(pitch)));
        }
    }
    else
    {
        m_firstMouseSample = true;
    }

    const double scrollDelta = window.consumeScrollDelta();
    if (controlsEnabled)
    {
        m_verticalFieldOfViewDegrees = glm::clamp(
            m_verticalFieldOfViewDegrees - static_cast<float>(scrollDelta) * 2.0f,
            20.0f,
            80.0f);
    }
}

void Camera::setMovementSpeed(float speed)
{
    if (speed <= 0.0f)
    {
        throw std::invalid_argument("Camera movement speed must be greater than zero.");
    }
    m_movementSpeed = speed;
}

void Camera::setMouseSensitivity(float sensitivity)
{
    if (sensitivity <= 0.0f)
    {
        throw std::invalid_argument("Camera mouse sensitivity must be greater than zero.");
    }
    m_mouseSensitivity = sensitivity;
}

const glm::vec3& Camera::position() const
{
    return m_position;
}

float Camera::fieldOfViewDegrees() const
{
    return m_verticalFieldOfViewDegrees;
}

void Camera::destroy()
{
    m_destroyed = true;
}

bool Camera::isDestroyed() const
{
    return m_destroyed;
}
