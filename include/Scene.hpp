#pragma once

#include "Transform.hpp"
#include "Physics.hpp"

#include <glm/vec3.hpp>

#include <optional>
#include <memory>
#include <string>
#include <unordered_map>
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
    float materialRoughness = 0.55f;
    float specularStrength = 0.35f;
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
        std::optional<PhysicsBody> physicsBody;
        std::string name;
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
    ScenePhysicsSettings& physicsSettings();
    const ScenePhysicsSettings& physicsSettings() const;
    unsigned int bindCube(
        Cube& cube,
        Shader& shader,
        const Transform& transform = {},
        std::optional<PhysicsBody> physicsBody = std::nullopt,
        const std::string& name = {});
    unsigned int bindSphere(
        Sphere& sphere,
        Shader& shader,
        const Transform& transform = {},
        std::optional<PhysicsBody> physicsBody = std::nullopt,
        const std::string& name = {});
    unsigned int bindPyramid(
        Pyramid& pyramid,
        Shader& shader,
        const Transform& transform = {},
        std::optional<PhysicsBody> physicsBody = std::nullopt,
        const std::string& name = {});
    unsigned int bindLight(
        LightSource& light,
        const Transform& transform = {},
        const std::string& name = {});
    unsigned int createPointLight(
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float radius = 8.0f,
        const Transform& transform = {},
        const std::string& name = {});

    const Camera& camera() const;
    const std::vector<Entity>& entities() const;
    std::vector<Entity>& entities();
    bool removeEntity(unsigned int id);
    void destroy();

private:
    std::vector<Entity> m_entities;
    std::unordered_map<unsigned int, std::unique_ptr<LightSource>> m_ownedLights;
    Camera* m_camera = nullptr;
    Cubemap* m_cubemap = nullptr;
    SceneLighting m_lighting;
    ScenePhysicsSettings m_physicsSettings;
    unsigned int m_nextEntityId = 1;
    bool m_destroyed = false;
};
