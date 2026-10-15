#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <vector>

class Shader;

struct LightGlow
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

class LightGlowRenderer
{
public:
    LightGlowRenderer();
    ~LightGlowRenderer();

    LightGlowRenderer(const LightGlowRenderer&) = delete;
    LightGlowRenderer& operator=(const LightGlowRenderer&) = delete;

    void render(
        const std::vector<LightGlow>& lights,
        const glm::mat4& view,
        const glm::mat4& projection) const;
    void destroy();

private:
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    std::unique_ptr<Shader> m_shader;
    bool m_destroyed = false;
};
