#include "LightGlowRenderer.hpp"

#include "Shader.hpp"

#include <glad/glad.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace
{
constexpr float vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
    -1.0f,  1.0f
};
}

LightGlowRenderer::LightGlowRenderer()
{
    m_shader = std::make_unique<Shader>(
        std::string(TRASHMAGIC_SHADER_DIR) + "/light_glow.vert",
        std::string(TRASHMAGIC_SHADER_DIR) + "/light_glow.frag");
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glBindVertexArray(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

LightGlowRenderer::~LightGlowRenderer()
{
    destroy();
}

void LightGlowRenderer::render(
    const std::vector<LightGlow>& lights,
    const glm::mat4& view,
    const glm::mat4& projection) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed light glow renderer.");
    }
    if (lights.empty())
    {
        return;
    }

    const GLboolean wasBlendingEnabled = glIsEnabled(GL_BLEND);
    GLint previousBlendSourceRgb = GL_ONE;
    GLint previousBlendDestinationRgb = GL_ZERO;
    GLint previousBlendSourceAlpha = GL_ONE;
    GLint previousBlendDestinationAlpha = GL_ZERO;
    GLboolean previousDepthMask = GL_TRUE;
    glGetIntegerv(GL_BLEND_SRC_RGB, &previousBlendSourceRgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &previousBlendDestinationRgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &previousBlendSourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &previousBlendDestinationAlpha);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);

    m_shader->use();
    m_shader->setMat4("uView", view);
    m_shader->setMat4("uProjection", projection);
    glBindVertexArray(m_vertexArray);
    for (const LightGlow& light : lights)
    {
        m_shader->setVec3("uPosition", light.position);
        m_shader->setVec3("uColor", light.color);
        m_shader->setFloat("uIntensity", light.intensity);
        m_shader->setFloat("uRadius", 0.45f + 0.2f * light.intensity);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);

    glDepthMask(previousDepthMask);
    glBlendFuncSeparate(
        static_cast<GLenum>(previousBlendSourceRgb),
        static_cast<GLenum>(previousBlendDestinationRgb),
        static_cast<GLenum>(previousBlendSourceAlpha),
        static_cast<GLenum>(previousBlendDestinationAlpha));
    if (wasBlendingEnabled == GL_FALSE)
    {
        glDisable(GL_BLEND);
    }
}

void LightGlowRenderer::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    if (m_vertexBuffer != 0)
    {
        glDeleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }
    if (m_vertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_vertexArray);
        m_vertexArray = 0;
    }
    m_shader.reset();
    m_destroyed = true;
}
