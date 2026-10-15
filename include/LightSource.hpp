#pragma once

#include <glm/vec3.hpp>

class LightSource
{
public:
    LightSource(const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    ~LightSource();

    LightSource(const LightSource&) = delete;
    LightSource& operator=(const LightSource&) = delete;

    const glm::vec3& color() const;
    float intensity() const;
    void destroy();
    bool isDestroyed() const;

private:
    glm::vec3 m_color;
    float m_intensity;
    bool m_destroyed = false;
};
