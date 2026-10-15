#pragma once

#include "Shader.hpp"

class Camera;
class Cubemap;
class Window;

class SkyboxRenderer
{
public:
    SkyboxRenderer();
    ~SkyboxRenderer();

    SkyboxRenderer(const SkyboxRenderer&) = delete;
    SkyboxRenderer& operator=(const SkyboxRenderer&) = delete;

    void render(
        const Cubemap& cubemap,
        const Camera& camera,
        float aspectRatio,
        float brightness) const;
    void destroy();

private:
    Shader m_shader;
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    bool m_destroyed = false;
};
