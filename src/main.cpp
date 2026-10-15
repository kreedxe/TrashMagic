
#include "Camera.hpp"
#include "Cube.hpp"
#include "Cubemap.hpp"
#include "Gui.hpp"
#include "InputEventSystem.hpp"
#include "LightSource.hpp"
#include "Physics.hpp"
#include "Pyramid.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "Sphere.hpp"
#include "Transform.hpp"
#include "Texture.hpp"
#include "Window.hpp"

#include <chrono>
#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include <glm/vec3.hpp>

int main()
{
    try
    {
        Window window(1280, 720, "TrashMagic Sandbox");
        window.setVSync(true);
        InputEventSystem inputEvents(window);
        Renderer renderer(window);
        Physics physics;
        Gui gui(window, renderer);
        inputEvents.setModeChangedCallback([&gui](InputMode mode)
        {
            gui.setInputEnabled(mode == InputMode::Gui);
        });

        Shader cubeShader(
            std::string(TRASHMAGIC_SHADER_DIR) + "/cube.vert",
            std::string(TRASHMAGIC_SHADER_DIR) + "/cube.frag");
        Texture plankTexture(std::string(TRASHMAGIC_TEXTURE_DIR) + "/plank.png");
        Texture prickTexture(std::string(TRASHMAGIC_TEXTURE_DIR) + "/prick.png");
        Cubemap environment(std::string(TRASHMAGIC_TEXTURE_DIR) + "/cubemap.png");

        Cube cubeA;
        Cube cubeB;
        Cube cubeC;
        Cube cubeD;
        Sphere sphere;
        Pyramid pyramid;
        cubeA.setTexture(plankTexture);
        cubeB.setTexture(prickTexture);
        cubeC.setTexture(prickTexture);
        cubeD.setTexture(plankTexture);
        sphere.setTexture(prickTexture);
        pyramid.setTexture(plankTexture);

        Camera camera(glm::vec3(0.0f, 4.0f, 14.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        LightSource warmLight(glm::vec3(1.0f, 0.48f, 0.22f), 1.1f);
        LightSource coolLight(glm::vec3(0.25f, 0.55f, 1.0f), 1.1f);
        LightSource pinkLight(glm::vec3(1.0f, 0.2f, 0.55f), 0.8f);

        Scene scene;
        scene.setCamera(camera);
        scene.setCubemap(environment);
        scene.lighting().ambientColor = glm::vec3(0.13f);
        scene.lighting().cubemapBrightness = 0.65f;
        scene.lighting().sunDirection = glm::vec3(-0.4f, -1.0f, -0.25f);
        scene.lighting().sunColor = glm::vec3(1.0f, 0.84f, 0.66f);
        scene.lighting().sunIntensity = 0.45f;

        Transform groundTransform;
        groundTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
        groundTransform.scale = glm::vec3(12.0f, 0.5f, 8.0f);
        scene.bindCube(
            cubeD,
            cubeShader,
            groundTransform,
            PhysicsBody{},
            "Ground");

        Transform cubeATransform;
        cubeATransform.position = glm::vec3(-2.0f, 3.0f, 0.0f);
        cubeATransform.scale = glm::vec3(1.0f);
        auto cubeABody = PhysicsBody::dynamic(1.2f);
        cubeABody.restitution = 0.12f;
        scene.bindCube(cubeA, cubeShader, cubeATransform, cubeABody, "Falling cube A");

        Transform cubeBTransform;
        cubeBTransform.position = glm::vec3(0.0f, 6.5f, 0.0f);
        cubeBTransform.scale = glm::vec3(0.9f);
        auto cubeBBody = PhysicsBody::dynamic(0.9f);
        cubeBBody.angularVelocity = glm::vec3(0.3f, 0.8f, 0.1f);
        scene.bindCube(cubeB, cubeShader, cubeBTransform, cubeBBody, "Falling cube B");

        Transform cubeCTransform;
        cubeCTransform.position = glm::vec3(2.0f, 4.5f, 0.0f);
        cubeCTransform.scale = glm::vec3(0.8f);
        auto cubeCBody = PhysicsBody::dynamic(0.8f);
        cubeCBody.restitution = 0.2f;
        scene.bindCube(cubeC, cubeShader, cubeCTransform, cubeCBody, "Falling cube C");

        Transform sphereTransform;
        sphereTransform.position = glm::vec3(-1.0f, 8.0f, 1.2f);
        sphereTransform.scale = glm::vec3(0.65f);
        auto sphereBody = PhysicsBody::dynamic(0.7f, PhysicsCollider::Sphere);
        sphereBody.restitution = 0.35f;
        scene.bindSphere(sphere, cubeShader, sphereTransform, sphereBody, "Bouncing sphere");

        Transform pyramidTransform;
        pyramidTransform.position = glm::vec3(1.4f, 7.5f, -1.3f);
        pyramidTransform.scale = glm::vec3(0.85f);
        auto pyramidBody = PhysicsBody::dynamic(1.0f);
        pyramidBody.angularVelocity = glm::vec3(0.0f, 0.4f, 0.0f);
        scene.bindPyramid(pyramid, cubeShader, pyramidTransform, pyramidBody, "Falling pyramid");

        Transform keyLightTransform;
        keyLightTransform.position = glm::vec3(-3.0f, 3.0f, 3.0f);
        scene.bindLight(warmLight, keyLightTransform, "Warm key light");

        Transform fillLightTransform;
        fillLightTransform.position = glm::vec3(3.0f, 1.5f, 1.0f);
        scene.bindLight(coolLight, fillLightTransform, "Cool fill light");

        Transform pinkLightTransform;
        pinkLightTransform.position = glm::vec3(0.0f, -2.0f, 2.5f);
        scene.bindLight(pinkLight, pinkLightTransform, "Pink accent light");

        using Clock = std::chrono::steady_clock;
        constexpr auto frameDuration = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / 60.0));
        auto nextFrameTime = Clock::now();
        auto previousFrameTime = nextFrameTime;
        float smoothedDeltaSeconds = 1.0f / 60.0f;

        while (!window.shouldClose())
        {
            window.pollEvents();

            const auto frameStart = Clock::now();
            const float deltaSeconds = std::chrono::duration<float>(
                frameStart - previousFrameTime).count();
            previousFrameTime = frameStart;
            camera.update(window, deltaSeconds, inputEvents.cameraControlsActive());
            physics.step(scene, deltaSeconds);
            renderer.render(scene);
            smoothedDeltaSeconds +=
                (deltaSeconds - smoothedDeltaSeconds) * (1.0f - std::exp(-deltaSeconds * 5.0f));
            gui.beginFrame();
            gui.drawEditor(
                1.0f / smoothedDeltaSeconds,
                smoothedDeltaSeconds * 1000.0f,
                camera,
                scene,
                cubeA,
                sphere,
                pyramid,
                cubeShader);
            gui.endFrame();
            window.swapBuffers();

            nextFrameTime += frameDuration;
            const auto now = Clock::now();
            if (now < nextFrameTime)
            {
                std::this_thread::sleep_until(nextFrameTime);
            }
            else
            {
                nextFrameTime = now;
            }
        }

        physics.removeScene(scene);
        scene.destroy();
        physics.destroy();
        warmLight.destroy();
        coolLight.destroy();
        pinkLight.destroy();
        camera.destroy();
        cubeA.destroy();
        cubeB.destroy();
        cubeC.destroy();
        cubeD.destroy();
        sphere.destroy();
        pyramid.destroy();
        plankTexture.destroy();
        prickTexture.destroy();
        environment.destroy();
        cubeShader.destroy();
        gui.destroy();
        inputEvents.destroy();
        renderer.destroy();
        window.destroy();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Application error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}