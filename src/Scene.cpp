#include "Scene.hpp"

#include "Camera.hpp"
#include "Cube.hpp"
#include "Cubemap.hpp"
#include "LightSource.hpp"
#include "Pyramid.hpp"
#include "Shader.hpp"
#include "Sphere.hpp"

#include <stdexcept>

Scene::~Scene()
{
    destroy();
}

void Scene::setCamera(Camera& camera)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (camera.isDestroyed())
    {
        throw std::invalid_argument("Cannot bind a destroyed camera to a scene.");
    }
    m_camera = &camera;
}

void Scene::setCubemap(Cubemap& cubemap)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (cubemap.isDestroyed())
    {
        throw std::invalid_argument("Cannot assign a destroyed cubemap to a scene.");
    }
    m_cubemap = &cubemap;
}

Cubemap* Scene::cubemap() const
{
    return m_cubemap;
}

SceneLighting& Scene::lighting()
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot access a destroyed scene.");
    }
    return m_lighting;
}

const SceneLighting& Scene::lighting() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot access a destroyed scene.");
    }
    return m_lighting;
}

unsigned int Scene::bindCube(Cube& cube, Shader& shader, const Transform& transform)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (cube.isDestroyed() || shader.isDestroyed())
    {
        throw std::invalid_argument("Cannot bind a destroyed cube or shader to a scene.");
    }

    const unsigned int id = m_nextEntityId++;
    m_entities.push_back(Entity{id, transform, &cube, &shader, nullptr, nullptr, nullptr});
    return id;
}

unsigned int Scene::bindSphere(Sphere& sphere, Shader& shader, const Transform& transform)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (sphere.isDestroyed() || shader.isDestroyed())
    {
        throw std::invalid_argument("Cannot bind a destroyed sphere or shader to a scene.");
    }

    const unsigned int id = m_nextEntityId++;
    m_entities.push_back(Entity{id, transform, nullptr, &shader, nullptr, &sphere, nullptr});
    return id;
}

unsigned int Scene::bindPyramid(Pyramid& pyramid, Shader& shader, const Transform& transform)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (pyramid.isDestroyed() || shader.isDestroyed())
    {
        throw std::invalid_argument("Cannot bind a destroyed pyramid or shader to a scene.");
    }

    const unsigned int id = m_nextEntityId++;
    m_entities.push_back(Entity{id, transform, nullptr, &shader, nullptr, nullptr, &pyramid});
    return id;
}

unsigned int Scene::bindLight(LightSource& light, const Transform& transform)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot modify a destroyed scene.");
    }
    if (light.isDestroyed())
    {
        throw std::invalid_argument("Cannot bind a destroyed light source to a scene.");
    }

    unsigned int lightCount = 0;
    for (const Entity& entity : m_entities)
    {
        if (entity.light != nullptr)
        {
            ++lightCount;
        }
    }
    if (lightCount >= Shader::MaxLights)
    {
        throw std::length_error("Scene exceeds the shader's light limit.");
    }

    const unsigned int id = m_nextEntityId++;
    m_entities.push_back(Entity{id, transform, nullptr, nullptr, &light, nullptr, nullptr});
    return id;
}

const Camera& Scene::camera() const
{
    if (m_destroyed || m_camera == nullptr)
    {
        throw std::logic_error("Scene has no active camera.");
    }
    return *m_camera;
}

const std::vector<Scene::Entity>& Scene::entities() const
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot access a destroyed scene.");
    }
    return m_entities;
}

void Scene::destroy()
{
    m_entities.clear();
    m_camera = nullptr;
    m_cubemap = nullptr;
    m_destroyed = true;
}
