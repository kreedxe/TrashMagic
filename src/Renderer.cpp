#include "Renderer.hpp"

#include "Camera.hpp"
#include "Cube.hpp"
#include "LightSource.hpp"
#include "LightGlowRenderer.hpp"
#include "Pyramid.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "ShadowMapRenderer.hpp"
#include "SkyboxRenderer.hpp"
#include "Sphere.hpp"
#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
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
    m_shadowMapRenderer = std::make_unique<ShadowMapRenderer>();
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
    if (!std::isfinite(settings.clearColor.r)
        || !std::isfinite(settings.clearColor.g)
        || !std::isfinite(settings.clearColor.b)
        || !std::isfinite(settings.clearColor.a)
        || settings.clearColor.r < 0.0f || settings.clearColor.r > 1.0f
        || settings.clearColor.g < 0.0f || settings.clearColor.g > 1.0f
        || settings.clearColor.b < 0.0f || settings.clearColor.b > 1.0f
        || settings.clearColor.a < 0.0f || settings.clearColor.a > 1.0f)
    {
        throw std::invalid_argument("Renderer clear color components must be within [0, 1].");
    }
    if (settings.drawMode != RendererDrawMode::Solid
        && settings.drawMode != RendererDrawMode::Wireframe
        && settings.drawMode != RendererDrawMode::Points)
    {
        throw std::invalid_argument("Unsupported renderer draw mode.");
    }
    m_settings = settings;
}

const RendererSettings& Renderer::settings() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot access settings on a destroyed renderer.");
    }
    return m_settings;
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    const float elapsedSeconds = std::chrono::duration<float>(
        std::chrono::steady_clock::now() - m_startTime).count();
    const float aspectRatio =
        static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    const glm::mat4 view = scene.camera().viewMatrix();
    const glm::mat4 projection = scene.camera().projectionMatrix(aspectRatio);

    std::vector<const Scene::Entity*> lights;
    lights.reserve(Shader::MaxLights);
    for (const Scene::Entity& entity : scene.entities())
    {
        if (entity.light != nullptr)
        {
            lights.push_back(&entity);
        }
    }

    glm::vec3 sunDirection = scene.lighting().sunDirection;
    if (glm::length(sunDirection) < 0.0001f)
    {
        sunDirection = glm::vec3(-0.4f, -1.0f, -0.25f);
    }
    sunDirection = glm::normalize(sunDirection);
    glm::vec3 boundsMinimum(0.0f);
    glm::vec3 boundsMaximum(0.0f);
    bool hasShadowCasters = false;
    for (const Scene::Entity& entity : scene.entities())
    {
        if (entity.shader == nullptr
            || (entity.cube == nullptr && entity.sphere == nullptr && entity.pyramid == nullptr))
        {
            continue;
        }

        const float shapeRadius = entity.sphere != nullptr
            ? 1.0f
            : (entity.pyramid != nullptr ? 0.7f : 0.5f);
        const float radius = shapeRadius * glm::length(glm::abs(entity.transform.scale));
        const glm::vec3 radiusVector(radius);
        if (!hasShadowCasters)
        {
            boundsMinimum = entity.transform.position - radiusVector;
            boundsMaximum = entity.transform.position + radiusVector;
            hasShadowCasters = true;
        }
        else
        {
            boundsMinimum = glm::min(boundsMinimum, entity.transform.position - radiusVector);
            boundsMaximum = glm::max(boundsMaximum, entity.transform.position + radiusVector);
        }
    }
    const glm::vec3 shadowCenter = hasShadowCasters
        ? (boundsMinimum + boundsMaximum) * 0.5f
        : scene.camera().position();
    const glm::vec3 sceneExtent = boundsMaximum - boundsMinimum;
    const float shadowExtent = hasShadowCasters
        ? glm::max(glm::max(sceneExtent.x, glm::max(sceneExtent.y, sceneExtent.z)) * 0.5f + 2.0f, 8.0f)
        : 16.0f;
    const float lightDistance = shadowExtent + 10.0f;
    const glm::vec3 lightUp = std::abs(glm::dot(sunDirection, glm::vec3(0.0f, 1.0f, 0.0f)))
            > 0.98f
        ? glm::vec3(0.0f, 0.0f, 1.0f)
        : glm::vec3(0.0f, 1.0f, 0.0f);
    const glm::mat4 lightView = glm::lookAt(
        shadowCenter - sunDirection * lightDistance,
        shadowCenter,
        lightUp);
    const glm::mat4 lightProjection = glm::ortho(
        -shadowExtent,
        shadowExtent,
        -shadowExtent,
        shadowExtent,
        0.1f,
        2.0f * lightDistance);
    const glm::mat4 lightSpace = lightProjection * lightView;

    if (m_settings.shadowsEnabled)
    {
        m_shadowMapRenderer->begin(lightSpace);
        for (const Scene::Entity& entity : scene.entities())
        {
            if (entity.shader == nullptr
                || (entity.cube == nullptr && entity.sphere == nullptr && entity.pyramid == nullptr))
            {
                continue;
            }
            m_shadowMapRenderer->draw(
                entity.transform.matrix(entity.physicsBody ? 0.0f : elapsedSeconds));
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
        m_shadowMapRenderer->end();
    }

    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(
        m_settings.clearColor.r,
        m_settings.clearColor.g,
        m_settings.clearColor.b,
        m_settings.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_settings.skyboxEnabled && scene.cubemap() != nullptr)
    {
        m_skyboxRenderer->render(
            *scene.cubemap(),
            scene.camera(),
            aspectRatio,
            scene.lighting().cubemapBrightness);
    }

    if (m_settings.shadowsEnabled)
    {
        m_shadowMapRenderer->bind(1);
    }
    const GLenum polygonMode = m_settings.drawMode == RendererDrawMode::Wireframe
        ? GL_LINE
        : (m_settings.drawMode == RendererDrawMode::Points ? GL_POINT : GL_FILL);
    if (m_settings.drawMode == RendererDrawMode::Points)
    {
        GLfloat pointSizeRange[2]{};
        glGetFloatv(GL_POINT_SIZE_RANGE, pointSizeRange);
        glPointSize(glm::clamp(6.0f, pointSizeRange[0], pointSizeRange[1]));
    }
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
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
        shader.setMat4(
            "uModel",
            entity.transform.matrix(entity.physicsBody ? 0.0f : elapsedSeconds));
        shader.setVec3(
            "uTextureScale",
            entity.cube != nullptr
                ? glm::abs(entity.transform.scale)
                : glm::vec3(1.0f));
        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setMat4("uLightSpace", lightSpace);
        shader.setInt("uTexture", 0);
        shader.setInt("uShadowMap", 1);
        shader.setInt("uShadowsEnabled", m_settings.shadowsEnabled ? 1 : 0);
        shader.setInt("uLightCount", static_cast<int>(lights.size()));
        shader.setVec3("uAmbientColor", scene.lighting().ambientColor);
        shader.setVec3("uSunDirection", sunDirection);
        shader.setVec3("uSunColor", scene.lighting().sunColor);
        shader.setFloat("uSunIntensity", scene.lighting().sunIntensity);
        shader.setVec3("uViewPosition", scene.camera().position());
        shader.setFloat("uRoughness", scene.lighting().materialRoughness);
        shader.setFloat("uSpecularStrength", scene.lighting().specularStrength);

        for (std::size_t index = 0; index < lights.size(); ++index)
        {
            const std::string suffix = "[" + std::to_string(index) + "]";
            const Scene::Entity& lightEntity = *lights[index];
            shader.setVec3("uLightPositions" + suffix, lightEntity.transform.position);
            shader.setVec3("uLightColors" + suffix, lightEntity.light->color());
            shader.setFloat("uLightIntensities" + suffix, lightEntity.light->intensity());
            shader.setFloat("uLightRadii" + suffix, lightEntity.light->radius());
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

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPointSize(1.0f);
    std::vector<LightGlow> lightGlows;
    lightGlows.reserve(lights.size());
    for (const Scene::Entity* lightEntity : lights)
    {
        lightGlows.push_back({
            lightEntity->transform.position,
            lightEntity->light->color(),
            lightEntity->light->intensity(),
            lightEntity->light->radius()});
    }
    if (m_settings.lightGlowsEnabled)
    {
        m_lightGlowRenderer->render(lightGlows, view, projection);
    }

    if (m_settings.lightMarkersEnabled)
    {
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
    m_shadowMapRenderer.reset();
    m_skyboxRenderer.reset();
    m_destroyed = true;
}

bool Renderer::isDestroyed() const
{
    return m_destroyed;
}
