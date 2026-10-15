#pragma once

#include "Transform.hpp"

#include <glm/vec3.hpp>

#include <vector>

class Camera;
class Cube;
class Cubemap;
class LightSource;
class Pyramid;
class Shader;
class Sphere;

struct SceneLighting
{
    float cubemapBrightness = 0.7f;
    glm::vec3 ambientColor{0.12f};
    glm::vec3 sunDirection{-0.4f, -1.0f, -0.25f};
    glm::vec3 sunColor{1.0f, 0.92f, 0.78f};
    float sunIntensity = 0.35f;
};

class Scene
{
public:
    struct Entity
    {
        unsigned int id;
        Transform transform;
        Cube* cube = nullptr;
        Shader* shader = nullptr;
        LightSource* light = nullptr;
        Sphere* sphere = nullptr;
        Pyramid* pyramid = nullptr;
    };

    Scene() = default;
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void setCamera(Camera& camera);
    void setCubemap(Cubemap& cubemap);
    Cubemap* cubemap() const;
    SceneLighting& lighting();
    const SceneLighting& lighting() const;
    unsigned int bindCube(Cube& cube, Shader& shader, const Transform& transform = {});
    unsigned int bindSphere(Sphere& sphere, Shader& shader, const Transform& transform = {});
    unsigned int bindPyramid(Pyramid& pyramid, Shader& shader, const Transform& transform = {});
    unsigned int bindLight(LightSource& light, const Transform& transform = {});

    const Camera& camera() const;
    const std::vector<Entity>& entities() const;
    void destroy();

private:
    std::vector<Entity> m_entities;
    Camera* m_camera = nullptr;
    Cubemap* m_cubemap = nullptr;
    SceneLighting m_lighting;
    unsigned int m_nextEntityId = 1;
    bool m_destroyed = false;
};
