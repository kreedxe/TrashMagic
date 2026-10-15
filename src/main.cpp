
#include "Camera.hpp"
#include "Cube.hpp"
#include "Cubemap.hpp"
#include "Gui.hpp"
#include "InputEventSystem.hpp"
#include "LightSource.hpp"
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

        Camera camera(glm::vec3(0.0f, 1.0f, 8.0f), glm::vec3(0.0f));
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

        Transform cubeATransform;
        cubeATransform.position = glm::vec3(-1.8f, 0.0f, 0.0f);
        cubeATransform.scale = glm::vec3(1.2f);
        cubeATransform.rotation = glm::vec3(0.25f, 0.35f, 0.0f);
        cubeATransform.angularVelocity = glm::vec3(0.25f, 0.45f, 0.1f);
        scene.bindCube(cubeA, cubeShader, cubeATransform);

        Transform cubeBTransform;
        cubeBTransform.position = glm::vec3(1.8f, 0.0f, 0.0f);
        cubeBTransform.scale = glm::vec3(1.1f);
        cubeBTransform.rotation = glm::vec3(-0.2f, 0.4f, 0.15f);
        cubeBTransform.angularVelocity = glm::vec3(-0.35f, 0.55f, 0.2f);
        scene.bindCube(cubeB, cubeShader, cubeBTransform);

        Transform cubeCTransform;
        cubeCTransform.position = glm::vec3(0.0f, 1.85f, -1.4f);
        cubeCTransform.scale = glm::vec3(0.75f);
        cubeCTransform.angularVelocity = glm::vec3(0.5f, -0.3f, 0.35f);
        scene.bindCube(cubeC, cubeShader, cubeCTransform);

        Transform cubeDTransform;
        cubeDTransform.position = glm::vec3(0.0f, -1.8f, 0.4f);
        cubeDTransform.scale = glm::vec3(0.8f);
        cubeDTransform.rotation = glm::vec3(0.2f, 0.0f, 0.3f);
        cubeDTransform.angularVelocity = glm::vec3(-0.3f, 0.25f, -0.4f);
        scene.bindCube(cubeD, cubeShader, cubeDTransform);

        Transform sphereTransform;
        sphereTransform.position = glm::vec3(-3.2f, 0.0f, -1.0f);
        sphereTransform.scale = glm::vec3(0.85f);
        sphereTransform.angularVelocity = glm::vec3(0.0f, 0.35f, 0.0f);
        scene.bindSphere(sphere, cubeShader, sphereTransform);

        Transform pyramidTransform;
        pyramidTransform.position = glm::vec3(3.2f, 0.0f, -1.0f);
        pyramidTransform.scale = glm::vec3(1.0f);
        pyramidTransform.rotation = glm::vec3(0.0f, 0.4f, 0.0f);
        pyramidTransform.angularVelocity = glm::vec3(0.2f, -0.3f, 0.0f);
        scene.bindPyramid(pyramid, cubeShader, pyramidTransform);

        Transform keyLightTransform;
        keyLightTransform.position = glm::vec3(-3.0f, 3.0f, 3.0f);
        scene.bindLight(warmLight, keyLightTransform);

        Transform fillLightTransform;
        fillLightTransform.position = glm::vec3(3.0f, 1.5f, 1.0f);
        scene.bindLight(coolLight, fillLightTransform);

        Transform pinkLightTransform;
        pinkLightTransform.position = glm::vec3(0.0f, -2.0f, 2.5f);
        scene.bindLight(pinkLight, pinkLightTransform);

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
            renderer.render(scene);
            smoothedDeltaSeconds +=
                (deltaSeconds - smoothedDeltaSeconds) * (1.0f - std::exp(-deltaSeconds * 5.0f));
            gui.beginFrame();
            gui.drawDebugOverlay(
                1.0f / smoothedDeltaSeconds,
                smoothedDeltaSeconds * 1000.0f,
                camera,
                scene);
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

        scene.destroy();
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