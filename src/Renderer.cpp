#include "Renderer.hpp"

#include "Camera.hpp"
#include "Cube.hpp"
#include "LightSource.hpp"
#include "LightGlowRenderer.hpp"
#include "Pyramid.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "SkyboxRenderer.hpp"
#include "Sphere.hpp"
#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/common.hpp>

#include <stdexcept>
#include <string>
#include <vector>

Renderer::Renderer(Window& window, const RendererSettings& settings)
    : m_window(window),
      m_settings(settings),
      m_startTime(std::chrono::steady_clock::now())
{
    if (m_window.getNativeHandle() == nullptr)
    {
        throw std::invalid_argument("Renderer requires an active window context.");
    }

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0)
    {
        throw std::runtime_error("Failed to initialize GLAD.");
    }

    if (GLAD_GL_VERSION_4_1 == 0)
    {
        throw std::runtime_error("OpenGL 4.1 is not available.");
    }

    m_skyboxRenderer = std::make_unique<SkyboxRenderer>();
    m_lightGlowRenderer = std::make_unique<LightGlowRenderer>();
    m_lightMarkerCube = std::make_unique<Cube>();
    m_lightMarkerShader = std::make_unique<Shader>(
        std::string(TRASHMAGIC_SHADER_DIR) + "/light_marker.vert",
        std::string(TRASHMAGIC_SHADER_DIR) + "/light_marker.frag");
    glEnable(GL_FRAMEBUFFER_SRGB);
}

Renderer::~Renderer()
{
    destroy();
}

void Renderer::setSettings(const RendererSettings& settings)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed renderer.");
    }
    m_settings = settings;
}

void Renderer::render(const Scene& scene) const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed renderer.");
    }
    if (m_window.getNativeHandle() == nullptr)
    {
        throw std::logic_error("Cannot render after the window has been destroyed.");
    }

    glClearColor(
        m_settings.clearColor.r,
        m_settings.clearColor.g,
        m_settings.clearColor.b,
        m_settings.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_settings.depthTesting)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    const int framebufferWidth = m_window.getFramebufferWidth();
    const int framebufferHeight = m_window.getFramebufferHeight();
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        return;
    }
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    const float elapsedSeconds = std::chrono::duration<float>(
        std::chrono::steady_clock::now() - m_startTime).count();
    const float aspectRatio =
        static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    const glm::mat4 view = scene.camera().viewMatrix();
    const glm::mat4 projection = scene.camera().projectionMatrix(aspectRatio);

    if (scene.cubemap() != nullptr)
    {
        m_skyboxRenderer->render(
            *scene.cubemap(),
            scene.camera(),
            aspectRatio,
            scene.lighting().cubemapBrightness);
    }

    std::vector<const Scene::Entity*> lights;
    lights.reserve(Shader::MaxLights);
    for (const Scene::Entity& entity : scene.entities())
    {
        if (entity.light != nullptr)
        {
            lights.push_back(&entity);
        }
    }

    for (const Scene::Entity& entity : scene.entities())
    {
        if (entity.cube == nullptr
            && entity.sphere == nullptr
            && entity.pyramid == nullptr)
        {
            continue;
        }
        if (entity.shader == nullptr)
        {
            continue;
        }

        Shader& shader = *entity.shader;
        shader.use();
        shader.setMat4("uModel", entity.transform.matrix(elapsedSeconds));
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setInt("uTexture", 0);
        shader.setInt("uLightCount", static_cast<int>(lights.size()));
        shader.setVec3("uAmbientColor", scene.lighting().ambientColor);
        shader.setVec3("uSunDirection", glm::normalize(scene.lighting().sunDirection));
        shader.setVec3("uSunColor", scene.lighting().sunColor);
        shader.setFloat("uSunIntensity", scene.lighting().sunIntensity);

        for (std::size_t index = 0; index < lights.size(); ++index)
        {
            const std::string suffix = "[" + std::to_string(index) + "]";
            const Scene::Entity& lightEntity = *lights[index];
            shader.setVec3("uLightPositions" + suffix, lightEntity.transform.position);
            shader.setVec3("uLightColors" + suffix, lightEntity.light->color());
            shader.setFloat("uLightIntensities" + suffix, lightEntity.light->intensity());
        }

        if (entity.cube != nullptr)
        {
            entity.cube->draw();
        }
        else if (entity.sphere != nullptr)
        {
            entity.sphere->draw();
        }
        else
        {
            entity.pyramid->draw();
        }
    }

    std::vector<LightGlow> lightGlows;
    lightGlows.reserve(lights.size());
    for (const Scene::Entity* lightEntity : lights)
    {
        lightGlows.push_back({
            lightEntity->transform.position,
            lightEntity->light->color(),
            lightEntity->light->intensity()});
    }
    m_lightGlowRenderer->render(lightGlows, view, projection);

    m_lightMarkerShader->use();
    m_lightMarkerShader->setMat4("uView", view);
    m_lightMarkerShader->setMat4("uProjection", projection);
    for (const Scene::Entity* lightEntity : lights)
    {
        Transform markerTransform = lightEntity->transform;
        markerTransform.scale = glm::vec3(0.12f);
        m_lightMarkerShader->setMat4("uModel", markerTransform.matrix());
        m_lightMarkerShader->setVec3(
            "uColor",
            lightEntity->light->color()
                * glm::min(lightEntity->light->intensity(), 1.0f));
        m_lightMarkerCube->draw();
    }
}

void Renderer::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    m_lightMarkerCube.reset();
    m_lightMarkerShader.reset();
    m_lightGlowRenderer.reset();
    m_skyboxRenderer.reset();
    m_destroyed = true;
}

bool Renderer::isDestroyed() const
{
    return m_destroyed;
}
