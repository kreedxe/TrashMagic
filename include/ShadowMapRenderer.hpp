#pragma once

#include <glm/mat4x4.hpp>

#include <memory>

class Shader;

class ShadowMapRenderer
{
public:
    ShadowMapRenderer();
    ~ShadowMapRenderer();

    ShadowMapRenderer(const ShadowMapRenderer&) = delete;
    ShadowMapRenderer& operator=(const ShadowMapRenderer&) = delete;

    void begin(const glm::mat4& lightSpace) const;
    void draw(const glm::mat4& model) const;
    void end() const;
    void bind(unsigned int textureUnit) const;
    void destroy();

private:
    unsigned int m_framebuffer = 0;
    unsigned int m_depthTexture = 0;
    std::unique_ptr<Shader> m_shader;
    mutable int m_previousFramebuffer = 0;
    mutable int m_previousViewport[4]{};
    mutable int m_previousCullFace = 0;
    mutable int m_previousDepthFunction = 0;
    mutable float m_previousPolygonOffsetFactor = 0.0f;
    mutable float m_previousPolygonOffsetUnits = 0.0f;
    mutable bool m_wasCullEnabled = false;
    mutable bool m_wasPolygonOffsetEnabled = false;
    mutable bool m_wasDepthTestEnabled = false;
    mutable unsigned char m_previousDepthWriteEnabled = 1;
    bool m_destroyed = false;
};
