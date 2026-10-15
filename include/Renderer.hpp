#pragma once

#include <glm/vec4.hpp>

#include <chrono>
#include <memory>

class Window;
class Scene;
class SkyboxRenderer;
class LightGlowRenderer;
class Cube;
class Shader;

struct RendererSettings
{
    glm::vec4 clearColor{0.08f, 0.08f, 0.1f, 1.0f};
    bool depthTesting = true;
};

class Renderer
{
public:
    explicit Renderer(Window& window, const RendererSettings& settings = {});
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void setSettings(const RendererSettings& settings);
    void render(const Scene& scene) const;
    void destroy();
    bool isDestroyed() const;

private:
    Window& m_window;
    RendererSettings m_settings;
    std::unique_ptr<SkyboxRenderer> m_skyboxRenderer;
    std::unique_ptr<LightGlowRenderer> m_lightGlowRenderer;
    std::unique_ptr<Cube> m_lightMarkerCube;
    std::unique_ptr<Shader> m_lightMarkerShader;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_destroyed = false;
};
