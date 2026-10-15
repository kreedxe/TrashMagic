#include "LightSource.hpp"

#include <glm/common.hpp>

#include <cmath>
#include <stdexcept>

LightSource::LightSource(const glm::vec3& color, float intensity, float radius)
    : m_color(color),
      m_intensity(intensity),
      m_radius(radius)
{
    if (!std::isfinite(intensity) || intensity < 0.0f)
    {
        throw std::invalid_argument("Light intensity must be finite and non-negative.");
    }
    if (!std::isfinite(radius) || radius <= 0.0f)
    {
        throw std::invalid_argument("Light radius must be finite and greater than zero.");
    }
}

LightSource::~LightSource()
{
    destroy();
}

const glm::vec3& LightSource::color() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed light source.");
    }
    return m_color;
}

float LightSource::intensity() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed light source.");
    }
    return m_intensity;
}

float LightSource::radius() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed light source.");
    }
    return m_radius;
}

void LightSource::setColor(const glm::vec3& color)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed light source.");
    }
    if (!std::isfinite(color.x) || !std::isfinite(color.y) || !std::isfinite(color.z)
        || glm::any(glm::lessThan(color, glm::vec3(0.0f))))
    {
        throw std::invalid_argument("Light color must be finite and non-negative.");
    }
    m_color = color;
}

void LightSource::setIntensity(float intensity)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed light source.");
    }
    if (!std::isfinite(intensity) || intensity < 0.0f)
    {
        throw std::invalid_argument("Light intensity must be finite and non-negative.");
    }
    m_intensity = intensity;
}

void LightSource::setRadius(float radius)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed light source.");
    }
    if (!std::isfinite(radius) || radius <= 0.0f)
    {
        throw std::invalid_argument("Light radius must be finite and greater than zero.");
    }
    m_radius = radius;
}

void LightSource::destroy()
{
    m_destroyed = true;
}

bool LightSource::isDestroyed() const
{
    return m_destroyed;
}
