#include "Physics.hpp"

#include "Scene.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace
{
constexpr JPH::ObjectLayer StaticLayer = 0;
constexpr JPH::ObjectLayer MovingLayer = 1;
constexpr JPH::uint LayerCount = 2;
constexpr JPH::uint MaxBodies = 4096;
constexpr JPH::uint MaxBodyPairs = 8192;
constexpr JPH::uint MaxContactConstraints = 4096;
constexpr float FixedTimeStep = 1.0f / 60.0f;
constexpr float MaxFrameTime = 0.25f;
constexpr int MaxSubsteps = 8;

std::mutex runtimeMutex;
unsigned int runtimeUsers = 0;

void acquireJoltRuntime()
{
    std::lock_guard<std::mutex> lock(runtimeMutex);
    if (runtimeUsers == 0)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
    }
    ++runtimeUsers;
}

void releaseJoltRuntime()
{
    std::lock_guard<std::mutex> lock(runtimeMutex);
    if (--runtimeUsers == 0)
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}

JPH::Vec3 toJolt(const glm::vec3& value)
{
    return {value.x, value.y, value.z};
}

JPH::Quat toJoltRotation(const glm::vec3& eulerRotation)
{
    const glm::quat orientation = glm::angleAxis(eulerRotation.x, glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::angleAxis(eulerRotation.y, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::angleAxis(eulerRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    return {orientation.x, orientation.y, orientation.z, orientation.w};
}

template <typename Vector>
glm::vec3 toGlm(const Vector& value)
{
    return {
        static_cast<float>(value.GetX()),
        static_cast<float>(value.GetY()),
        static_cast<float>(value.GetZ())};
}

glm::vec3 toEuler(const JPH::Quat& value)
{
    const glm::quat rotation(
        value.GetW(), value.GetX(), value.GetY(), value.GetZ());
    const float r00 = 1.0f - 2.0f * (rotation.y * rotation.y + rotation.z * rotation.z);
    const float r01 = 2.0f * (rotation.x * rotation.y - rotation.z * rotation.w);
    const float r02 = 2.0f * (rotation.x * rotation.z + rotation.y * rotation.w);
    const float r12 = 2.0f * (rotation.y * rotation.z - rotation.x * rotation.w);
    const float r22 = 1.0f - 2.0f * (rotation.x * rotation.x + rotation.y * rotation.y);
    return {
        std::atan2(-r12, r22),
        std::asin(glm::clamp(r02, -1.0f, 1.0f)),
        std::atan2(-r01, r00)};
}

JPH::EMotionType toJoltMotionType(PhysicsMotionType type)
{
    switch (type)
    {
    case PhysicsMotionType::Static:
        return JPH::EMotionType::Static;
    case PhysicsMotionType::Dynamic:
        return JPH::EMotionType::Dynamic;
    case PhysicsMotionType::Kinematic:
        return JPH::EMotionType::Kinematic;
    }
    throw std::invalid_argument("Unsupported physics motion type.");
}

bool isFinite(const glm::vec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

void validateBody(const Scene::Entity& entity, const PhysicsBody& body)
{
    if (!isFinite(entity.transform.position)
        || !isFinite(entity.transform.rotation)
        || !isFinite(entity.transform.scale)
        || !isFinite(body.velocity)
        || !isFinite(body.angularVelocity))
    {
        throw std::invalid_argument("Physics body transforms and velocities must be finite.");
    }
    if (glm::any(glm::lessThanEqual(glm::abs(entity.transform.scale), glm::vec3(0.0f))))
    {
        throw std::invalid_argument("Physics body scale must be non-zero on every axis.");
    }
    if (!std::isfinite(body.mass)
        || !std::isfinite(body.friction)
        || !std::isfinite(body.restitution)
        || !std::isfinite(body.gravityScale)
        || body.friction < 0.0f
        || body.restitution < 0.0f
        || body.restitution > 1.0f
        || (body.motionType == PhysicsMotionType::Dynamic && body.mass <= 0.0f))
    {
        throw std::invalid_argument("Physics body mass or material settings are invalid.");
    }
}

struct JoltRuntime
{
    JoltRuntime()
    {
        acquireJoltRuntime();
    }

    ~JoltRuntime()
    {
        releaseJoltRuntime();
    }
};

struct SceneWorld
{
    struct BodyRecord
    {
        JPH::BodyID id;
        PhysicsBody settings;
        glm::vec3 scale;
        glm::vec3 lastPosition;
        glm::vec3 lastRotation;
        glm::vec3 lastVelocity;
        glm::vec3 lastAngularVelocity;
    };

    explicit SceneWorld(
        const JPH::BroadPhaseLayerInterface& broadPhaseLayers,
        const JPH::ObjectVsBroadPhaseLayerFilter& objectVsBroadPhase,
        const JPH::ObjectLayerPairFilter& objectLayerPair,
        JPH::JobSystem& jobSystem)
        : jobs(jobSystem)
    {
        system.Init(
            MaxBodies,
            0,
            MaxBodyPairs,
            MaxContactConstraints,
            broadPhaseLayers,
            objectVsBroadPhase,
            objectLayerPair);
    }

    ~SceneWorld()
    {
        JPH::BodyInterface& bodyInterface = system.GetBodyInterface();
        for (const auto& [entityId, body] : bodies)
        {
            static_cast<void>(entityId);
            bodyInterface.RemoveBody(body.id);
            bodyInterface.DestroyBody(body.id);
        }
    }

    JPH::PhysicsSystem system;
    JPH::JobSystem& jobs;
    std::unordered_map<unsigned int, BodyRecord> bodies;
    float accumulator = 0.0f;
};
}

class Physics::Impl
{
public:
    Impl()
        : objectVsBroadPhase(nullptr),
          jobs(
              JPH::cMaxPhysicsJobs,
              JPH::cMaxPhysicsBarriers,
              std::max(1, std::min(4, static_cast<int>(std::thread::hardware_concurrency()) - 1))),
          tempAllocator(10 * 1024 * 1024)
    {
        broadPhaseLayers.MapObjectToBroadPhaseLayer(StaticLayer, JPH::BroadPhaseLayer(0));
        broadPhaseLayers.MapObjectToBroadPhaseLayer(MovingLayer, JPH::BroadPhaseLayer(1));
        objectLayerPair.EnableCollision(StaticLayer, MovingLayer);
        objectLayerPair.EnableCollision(MovingLayer, MovingLayer);
        objectVsBroadPhase = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
            broadPhaseLayers,
            LayerCount,
            objectLayerPair,
            LayerCount);
    }

    void step(Scene& scene, float deltaSeconds)
    {
        if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.0f)
        {
            throw std::invalid_argument("Physics delta time must be finite and non-negative.");
        }
        if (!isFinite(scene.physicsSettings().gravity))
        {
            throw std::invalid_argument("Scene gravity must be finite.");
        }

        SceneWorld& world = getWorld(scene);
        synchronizeNewBodies(scene, world);
        synchronizeEditedEntities(scene, world);
        world.system.SetGravity(toJolt(scene.physicsSettings().gravity));
        world.accumulator = std::min(
            world.accumulator + std::min(deltaSeconds, MaxFrameTime),
            FixedTimeStep * static_cast<float>(MaxSubsteps));

        while (world.accumulator >= FixedTimeStep)
        {
            world.system.Update(FixedTimeStep, 1, &tempAllocator, &world.jobs);
            world.accumulator -= FixedTimeStep;
        }

        synchronizeTransforms(scene, world);
    }

    void removeScene(Scene& scene)
    {
        worlds.erase(&scene);
    }

private:
    SceneWorld& getWorld(Scene& scene)
    {
        auto found = worlds.find(&scene);
        if (found != worlds.end())
        {
            return *found->second;
        }

        auto world = std::make_unique<SceneWorld>(
            broadPhaseLayers,
            *objectVsBroadPhase,
            objectLayerPair,
            jobs);
        SceneWorld& reference = *world;
        worlds.emplace(&scene, std::move(world));
        return reference;
    }

    void synchronizeNewBodies(Scene& scene, SceneWorld& world)
    {
        JPH::BodyInterface& bodyInterface = world.system.GetBodyInterface();
        std::unordered_set<unsigned int> activeBodyIds;
        activeBodyIds.reserve(scene.entities().size());
        for (const Scene::Entity& entity : scene.entities())
        {
            if (entity.physicsBody)
            {
                activeBodyIds.insert(entity.id);
            }
        }

        for (auto body = world.bodies.begin(); body != world.bodies.end();)
        {
            if (!activeBodyIds.contains(body->first))
            {
                bodyInterface.RemoveBody(body->second.id);
                bodyInterface.DestroyBody(body->second.id);
                body = world.bodies.erase(body);
            }
            else
            {
                ++body;
            }
        }

        for (Scene::Entity& entity : scene.entities())
        {
            if (!entity.physicsBody)
            {
                continue;
            }

            const PhysicsBody& body = *entity.physicsBody;
            validateBody(entity, body);
            const glm::vec3 scale = glm::abs(entity.transform.scale);
            auto existing = world.bodies.find(entity.id);
            if (existing != world.bodies.end())
            {
                const PhysicsBody& previous = existing->second.settings;
                const bool configurationChanged =
                    previous.motionType != body.motionType
                    || previous.collider != body.collider
                    || previous.mass != body.mass
                    || previous.friction != body.friction
                    || previous.restitution != body.restitution
                    || previous.gravityScale != body.gravityScale
                    || previous.solid != body.solid
                    || existing->second.scale != scale;
                if (!configurationChanged)
                {
                    continue;
                }
                bodyInterface.RemoveBody(existing->second.id);
                bodyInterface.DestroyBody(existing->second.id);
                world.bodies.erase(existing);
            }

            JPH::Shape* shape = nullptr;
            if (body.collider == PhysicsCollider::Sphere)
            {
                shape = new JPH::SphereShape(std::max(scale.x, std::max(scale.y, scale.z)));
            }
            else
            {
                shape = new JPH::BoxShape(
                    JPH::Vec3(0.5f * scale.x, 0.5f * scale.y, 0.5f * scale.z));
            }

            const JPH::ObjectLayer layer =
                body.motionType == PhysicsMotionType::Static ? StaticLayer : MovingLayer;
            JPH::BodyCreationSettings settings(
                shape,
                JPH::RVec3(
                    entity.transform.position.x,
                    entity.transform.position.y,
                    entity.transform.position.z),
                toJoltRotation(entity.transform.rotation),
                toJoltMotionType(body.motionType),
                layer);
            settings.mIsSensor = !body.solid;
            settings.mFriction = body.friction;
            settings.mRestitution = body.restitution;
            settings.mGravityFactor = body.gravityScale;
            settings.mLinearVelocity = toJolt(body.velocity);
            settings.mAngularVelocity = toJolt(body.angularVelocity);
            if (body.motionType == PhysicsMotionType::Dynamic)
            {
                settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
                settings.mMassPropertiesOverride.mMass = body.mass;
            }

            const JPH::EActivation activation =
                body.motionType == PhysicsMotionType::Static
                    ? JPH::EActivation::DontActivate
                    : JPH::EActivation::Activate;
            const JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(settings, activation);
            if (bodyId.IsInvalid())
            {
                throw std::runtime_error("Physics engine could not create a body for a scene entity.");
            }
            world.bodies.emplace(
                entity.id,
                SceneWorld::BodyRecord{
                    bodyId,
                    body,
                    scale,
                    entity.transform.position,
                    entity.transform.rotation,
                    body.velocity,
                    body.angularVelocity});
        }
    }

    void synchronizeEditedEntities(Scene& scene, SceneWorld& world)
    {
        JPH::BodyInterface& bodyInterface = world.system.GetBodyInterface();
        for (Scene::Entity& entity : scene.entities())
        {
            if (!entity.physicsBody)
            {
                continue;
            }
            const auto body = world.bodies.find(entity.id);
            if (body == world.bodies.end())
            {
                continue;
            }

            SceneWorld::BodyRecord& record = body->second;
            const bool positionEdited =
                glm::length(entity.transform.position - record.lastPosition) > 0.001f;
            const bool rotationEdited =
                glm::length(entity.transform.rotation - record.lastRotation) > 0.001f;
            if (positionEdited || rotationEdited)
            {
                bodyInterface.SetPositionAndRotation(
                    record.id,
                    JPH::RVec3(
                        entity.transform.position.x,
                        entity.transform.position.y,
                        entity.transform.position.z),
                    toJoltRotation(entity.transform.rotation),
                    JPH::EActivation::Activate);
                record.lastPosition = entity.transform.position;
                record.lastRotation = entity.transform.rotation;
            }

            if (entity.physicsBody->motionType == PhysicsMotionType::Dynamic)
            {
                if (glm::length(entity.physicsBody->velocity - record.lastVelocity) > 0.001f)
                {
                    bodyInterface.SetLinearVelocity(record.id, toJolt(entity.physicsBody->velocity));
                    record.lastVelocity = entity.physicsBody->velocity;
                }
                if (glm::length(
                        entity.physicsBody->angularVelocity - record.lastAngularVelocity) > 0.001f)
                {
                    bodyInterface.SetAngularVelocity(
                        record.id,
                        toJolt(entity.physicsBody->angularVelocity));
                    record.lastAngularVelocity = entity.physicsBody->angularVelocity;
                }
            }
        }
    }

    void synchronizeTransforms(Scene& scene, SceneWorld& world)
    {
        JPH::BodyInterface& bodyInterface = world.system.GetBodyInterface();
        for (Scene::Entity& entity : scene.entities())
        {
            if (!entity.physicsBody)
            {
                continue;
            }
            const auto body = world.bodies.find(entity.id);
            if (body == world.bodies.end())
            {
                continue;
            }

            JPH::RVec3 position;
            JPH::Quat rotation;
            bodyInterface.GetPositionAndRotation(body->second.id, position, rotation);
            if (entity.physicsBody->motionType == PhysicsMotionType::Static)
            {
                body->second.lastPosition = entity.transform.position;
                body->second.lastRotation = entity.transform.rotation;
                continue;
            }
            entity.transform.position = toGlm(position);
            entity.transform.rotation = toEuler(rotation);
            entity.physicsBody->velocity = toGlm(bodyInterface.GetLinearVelocity(body->second.id));
            const JPH::Vec3 angularVelocity = bodyInterface.GetAngularVelocity(body->second.id);
            entity.physicsBody->angularVelocity = {
                angularVelocity.GetX(),
                angularVelocity.GetY(),
                angularVelocity.GetZ()};
            body->second.lastPosition = entity.transform.position;
            body->second.lastRotation = entity.transform.rotation;
            body->second.lastVelocity = entity.physicsBody->velocity;
            body->second.lastAngularVelocity = entity.physicsBody->angularVelocity;
        }
    }

    JoltRuntime runtime;
    JPH::BroadPhaseLayerInterfaceTable broadPhaseLayers{LayerCount, LayerCount};
    JPH::ObjectLayerPairFilterTable objectLayerPair{LayerCount};
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> objectVsBroadPhase;
    JPH::JobSystemThreadPool jobs;
    JPH::TempAllocatorImpl tempAllocator;
    std::unordered_map<Scene*, std::unique_ptr<SceneWorld>> worlds;
};

PhysicsBody PhysicsBody::dynamic(float mass, PhysicsCollider collider)
{
    PhysicsBody body;
    body.motionType = PhysicsMotionType::Dynamic;
    body.collider = collider;
    body.mass = mass;
    return body;
}

Physics::Physics()
    : m_impl(std::make_unique<Impl>())
{
}

Physics::~Physics()
{
    destroy();
}

void Physics::step(Scene& scene, float deltaSeconds)
{
    if (m_destroyed || !m_impl)
    {
        throw std::logic_error("Cannot step a destroyed physics engine.");
    }
    m_impl->step(scene, deltaSeconds);
}

void Physics::removeScene(Scene& scene)
{
    if (m_impl)
    {
        m_impl->removeScene(scene);
    }
}

void Physics::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    m_impl.reset();
    m_destroyed = true;
}

bool Physics::isDestroyed() const
{
    return m_destroyed;
}
