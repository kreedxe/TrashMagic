#include "SkyboxRenderer.hpp"

#include "Camera.hpp"
#include "Cubemap.hpp"

#include <glad/glad.h>

#include <glm/mat4x4.hpp>

#include <stdexcept>
#include <string>

namespace
{
constexpr float vertices[] = {
    -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
};
}

SkyboxRenderer::SkyboxRenderer()
    : m_shader(
          std::string(TRASHMAGIC_SHADER_DIR) + "/skybox.vert",
          std::string(TRASHMAGIC_SHADER_DIR) + "/skybox.frag")
{
    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);
    glBindVertexArray(m_vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

SkyboxRenderer::~SkyboxRenderer()
{
    destroy();
}

void SkyboxRenderer::render(
    const Cubemap& cubemap,
    const Camera& camera,
    float aspectRatio,
    float brightness) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed skybox renderer.");
    }

    GLint previousDepthFunction = GL_LESS;
    GLboolean previousDepthMask = GL_TRUE;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 view = camera.viewMatrix();
    view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    m_shader.use();
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", camera.projectionMatrix(aspectRatio));
    m_shader.setFloat("uBrightness", brightness);
    m_shader.setInt("uCubemap", 0);
    cubemap.bind(0);

    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(previousDepthMask);
    glDepthFunc(static_cast<GLenum>(previousDepthFunction));
}

void SkyboxRenderer::destroy()
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
    m_shader.destroy();
    m_destroyed = true;
}
