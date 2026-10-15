#pragma once

#include <glm/vec3.hpp>

#include <memory>

class Scene;

enum class PhysicsMotionType
{
    Static,
    Dynamic,
    Kinematic
};

enum class PhysicsCollider
{
    Box,
    Sphere
};

struct PhysicsBody
{
    PhysicsMotionType motionType = PhysicsMotionType::Static;
    PhysicsCollider collider = PhysicsCollider::Box;
    float mass = 1.0f;
    float friction = 0.6f;
    float restitution = 0.05f;
    float gravityScale = 1.0f;
    bool solid = true;
    glm::vec3 velocity{0.0f};
    glm::vec3 angularVelocity{0.0f};

    static PhysicsBody dynamic(
        float mass = 1.0f,
        PhysicsCollider collider = PhysicsCollider::Box);
};

struct ScenePhysicsSettings
{
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};
};

class Physics
{
public:
    Physics();
    ~Physics();

    Physics(const Physics&) = delete;
    Physics& operator=(const Physics&) = delete;

    void step(Scene& scene, float deltaSeconds);
    void removeScene(Scene& scene);
    void destroy();
    bool isDestroyed() const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
    bool m_destroyed = false;
};
