#include "LightSource.hpp"

#include <stdexcept>

LightSource::LightSource(const glm::vec3& color, float intensity)
    : m_color(color),
      m_intensity(intensity)
{
    if (intensity < 0.0f)
    {
        throw std::invalid_argument("Light intensity must be non-negative.");
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

void LightSource::destroy()
{
    m_destroyed = true;
}

bool LightSource::isDestroyed() const
{
    return m_destroyed;
}
