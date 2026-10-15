#include "ShadowMapRenderer.hpp"

#include "Shader.hpp"

#include <glad/glad.h>

#include <stdexcept>
#include <string>

namespace
{
constexpr int ShadowMapSize = 2048;
}

ShadowMapRenderer::ShadowMapRenderer()
    : m_shader(std::make_unique<Shader>(
          std::string(TRASHMAGIC_SHADER_DIR) + "/shadow_depth.vert",
          std::string(TRASHMAGIC_SHADER_DIR) + "/shadow_depth.frag"))
{
    GLint previousTexture = 0;
    GLint previousFramebuffer = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);

    glGenFramebuffers(1, &m_framebuffer);
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        ShadowMapSize,
        ShadowMapSize,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    constexpr float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_depthTexture,
        0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
        destroy();
        throw std::runtime_error("Failed to create the directional shadow framebuffer.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
}

ShadowMapRenderer::~ShadowMapRenderer()
{
    destroy();
}

void ShadowMapRenderer::begin(const glm::mat4& lightSpace) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed shadow map renderer.");
    }
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_previousFramebuffer);
    glGetIntegerv(GL_VIEWPORT, m_previousViewport);
    glGetIntegerv(GL_CULL_FACE_MODE, &m_previousCullFace);
    glGetIntegerv(GL_DEPTH_FUNC, &m_previousDepthFunction);
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &m_previousPolygonOffsetFactor);
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &m_previousPolygonOffsetUnits);
    m_wasCullEnabled = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
    m_wasPolygonOffsetEnabled = glIsEnabled(GL_POLYGON_OFFSET_FILL) == GL_TRUE;
    m_wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &m_previousDepthWriteEnabled);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, ShadowMapSize, ShadowMapSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
    m_shader->use();
    m_shader->setMat4("uLightSpace", lightSpace);
}

void ShadowMapRenderer::draw(const glm::mat4& model) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed shadow map renderer.");
    }
    m_shader->setMat4("uModel", model);
}

void ShadowMapRenderer::end() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(m_previousFramebuffer));
    glViewport(
        m_previousViewport[0],
        m_previousViewport[1],
        m_previousViewport[2],
        m_previousViewport[3]);
    glCullFace(static_cast<GLenum>(m_previousCullFace));
    glDepthFunc(static_cast<GLenum>(m_previousDepthFunction));
    glDepthMask(m_previousDepthWriteEnabled);
    glPolygonOffset(m_previousPolygonOffsetFactor, m_previousPolygonOffsetUnits);
    if (!m_wasCullEnabled)
    {
        glDisable(GL_CULL_FACE);
    }
    if (!m_wasPolygonOffsetEnabled)
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    if (!m_wasDepthTestEnabled)
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void ShadowMapRenderer::bind(unsigned int textureUnit) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot bind a destroyed shadow map.");
    }
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}

void ShadowMapRenderer::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    if (m_depthTexture != 0)
    {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }
    if (m_framebuffer != 0)
    {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    m_shader.reset();
    m_destroyed = true;
}
