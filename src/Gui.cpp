#include "Gui.hpp"

#include "Camera.hpp"
#include "Cube.hpp"
#include "LightSource.hpp"
#include "Pyramid.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Shader.hpp"
#include "Sphere.hpp"
#include "Transform.hpp"
#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/common.hpp>
#include <glm/geometric.hpp>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>

namespace
{
void drawSceneInspector(Scene& scene, const Camera& camera)
{
    ImGui::TextUnformatted("Scene");
    ImGui::Separator();

    ScenePhysicsSettings& physics = scene.physicsSettings();
    if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Gravity", &physics.gravity.x, 0.05f, -50.0f, 50.0f, "%.2f");
    }

    SceneLighting& lighting = scene.lighting();
    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Environment", &lighting.cubemapBrightness, 0.0f, 3.0f);
        ImGui::ColorEdit3("Ambient", &lighting.ambientColor.x);
        ImGui::DragFloat3(
            "Sun direction",
            &lighting.sunDirection.x,
            0.02f,
            -1.0f,
            1.0f);
        if (glm::length(lighting.sunDirection) > 0.0001f)
        {
            lighting.sunDirection = glm::normalize(lighting.sunDirection);
        }
        else
        {
            lighting.sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        }
        ImGui::ColorEdit3("Sun color", &lighting.sunColor.x);
        ImGui::SliderFloat("Sun intensity", &lighting.sunIntensity, 0.0f, 5.0f);
        ImGui::SliderFloat("Roughness", &lighting.materialRoughness, 0.04f, 1.0f);
        ImGui::SliderFloat("Specular", &lighting.specularStrength, 0.0f, 1.0f);
    }

    ImGui::Text("Camera FOV: %.1f deg", camera.fieldOfViewDegrees());
    ImGui::Text("Environment: %s", scene.cubemap() ? "assigned" : "none");
}

void drawEntityInspector(Scene::Entity& entity)
{
    char nameBuffer[128]{};
    std::strncpy(nameBuffer, entity.name.c_str(), sizeof(nameBuffer) - 1);
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
    {
        entity.name = nameBuffer;
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::DragFloat3("Position", &entity.transform.position.x, 0.05f);
        ImGui::DragFloat3("Rotation", &entity.transform.rotation.x, 0.02f);
        ImGui::DragFloat3(
            "Scale",
            &entity.transform.scale.x,
            0.02f,
            0.01f,
            100.0f);
    }

    if (entity.light != nullptr)
    {
        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 color = entity.light->color();
            if (ImGui::ColorEdit3("Color", &color.x))
            {
                entity.light->setColor(color);
            }
            float intensity = entity.light->intensity();
            if (ImGui::DragFloat("Intensity", &intensity, 0.02f, 0.0f, 20.0f))
            {
                entity.light->setIntensity(intensity);
            }
            float radius = entity.light->radius();
            if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.1f, 100.0f, "%.1f"))
            {
                entity.light->setRadius(radius);
            }
        }
    }

    if (entity.physicsBody)
    {
        if (ImGui::CollapsingHeader("Physics Body", ImGuiTreeNodeFlags_DefaultOpen))
        {
            PhysicsBody& body = *entity.physicsBody;
            int motionType = static_cast<int>(body.motionType);
            const char* motionTypes[] = {"Static", "Dynamic", "Kinematic"};
            if (ImGui::Combo("Motion type", &motionType, motionTypes, 3))
            {
                body.motionType = static_cast<PhysicsMotionType>(motionType);
            }

            int collider = static_cast<int>(body.collider);
            const char* colliders[] = {"Box", "Sphere"};
            if (ImGui::Combo("Collider", &collider, colliders, 2))
            {
                body.collider = static_cast<PhysicsCollider>(collider);
            }
            ImGui::DragFloat("Mass", &body.mass, 0.05f, 0.05f, 1000.0f);
            ImGui::SliderFloat("Friction", &body.friction, 0.0f, 2.0f);
            ImGui::SliderFloat("Restitution", &body.restitution, 0.0f, 1.0f);
            ImGui::DragFloat("Gravity scale", &body.gravityScale, 0.02f, -5.0f, 5.0f);
            ImGui::Checkbox("Solid", &body.solid);
            ImGui::DragFloat3("Velocity", &body.velocity.x, 0.05f);
        }
    }
    else if (entity.light == nullptr)
    {
        ImGui::Separator();
        ImGui::TextUnformatted("No physics body");
    }
}

void applyEditorTheme()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(7.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 13.0f;
    style.WindowRounding = 0.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.88f, 0.91f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.53f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.075f, 0.090f, 0.115f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.060f, 0.073f, 0.095f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.095f, 0.112f, 0.140f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.16f, 0.19f, 0.23f, 0.70f);
    colors[ImGuiCol_Separator] = ImVec4(0.18f, 0.22f, 0.27f, 0.80f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.27f, 0.34f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.055f, 0.068f, 0.088f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.055f, 0.068f, 0.088f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.055f, 0.068f, 0.088f, 0.70f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.23f, 0.28f, 0.34f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.31f, 0.39f, 0.47f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.48f, 0.58f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.36f, 0.75f, 0.94f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.29f, 0.65f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.78f, 0.97f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.20f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.31f, 0.40f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.23f, 0.39f, 0.50f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.13f, 0.23f, 0.31f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.31f, 0.41f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.19f, 0.37f, 0.49f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.14f, 0.19f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.34f, 0.45f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.15f, 0.27f, 0.36f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.36f, 0.75f, 0.94f, 1.00f);
}
}

Gui::Gui(Window& window, Renderer& renderer)
    : m_window(window),
      m_renderer(renderer)
{
    if (m_window.getNativeHandle() == nullptr || m_renderer.isDestroyed())
    {
        throw std::invalid_argument("GUI requires an active window and renderer.");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    applyEditorTheme();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImFontConfig fontConfig;
    fontConfig.OversampleH = 2;
    fontConfig.OversampleV = 2;
    if (ImGui::GetIO().Fonts->AddFontFromFileTTF(
            TRASHMAGIC_FONT_PATH,
            16.0f,
            &fontConfig) == nullptr)
    {
        ImGui::DestroyContext();
        throw std::runtime_error(
            std::string("Failed to load editor font: ") + TRASHMAGIC_FONT_PATH);
    }

    if (!ImGui_ImplGlfw_InitForOpenGL(m_window.getNativeHandle(), true))
    {
        ImGui::DestroyContext();
        throw std::runtime_error("Failed to initialize the ImGui GLFW backend.");
    }

    if (!ImGui_ImplOpenGL3_Init("#version 410"))
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        throw std::runtime_error("Failed to initialize the ImGui OpenGL backend.");
    }
}

Gui::~Gui()
{
    destroy();
}

void Gui::beginFrame()
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot use a destroyed GUI.");
    }
    if (m_frameStarted)
    {
        throw std::logic_error("GUI frame has already started.");
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    m_frameStarted = true;
}

void Gui::setInputEnabled(bool enabled)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot change input on a destroyed GUI.");
    }
    m_inputEnabled = enabled;
}

void Gui::drawEditor(
    float framesPerSecond,
    float frameMilliseconds,
    Camera& camera,
    Scene& scene,
    Cube& cubeTemplate,
    Sphere& sphereTemplate,
    Pyramid& pyramidTemplate,
    Shader& shader)
{
    if (m_destroyed || !m_frameStarted)
    {
        throw std::logic_error("Begin the GUI frame before drawing the editor.");
    }

    if (ImGui::BeginMainMenuBar())
    {
        ImGui::TextUnformatted("TrashMagic");
        ImGui::SameLine();
        ImGui::TextDisabled("3D EDITOR");
        ImGui::Separator();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
                m_window.requestClose();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Scene Hierarchy", nullptr, &m_showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
            ImGui::MenuItem("Renderer Configuration", nullptr, &m_showRendererConfig);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create", m_inputEnabled))
        {
            const glm::vec3 spawnPosition = camera.position() + camera.forward() * 4.0f;
            Transform spawnTransform;
            spawnTransform.position = spawnPosition;
            if (ImGui::MenuItem("Cube"))
            {
                m_selectedEntityId = scene.bindCube(
                    cubeTemplate,
                    shader,
                    spawnTransform,
                    PhysicsBody::dynamic());
            }
            if (ImGui::MenuItem("Sphere"))
            {
                m_selectedEntityId = scene.bindSphere(
                    sphereTemplate,
                    shader,
                    spawnTransform,
                    PhysicsBody::dynamic(1.0f, PhysicsCollider::Sphere));
            }
            if (ImGui::MenuItem("Pyramid"))
            {
                m_selectedEntityId = scene.bindPyramid(
                    pyramidTemplate,
                    shader,
                    spawnTransform,
                    PhysicsBody::dynamic());
            }

            std::size_t pointLightCount = 0;
            for (const Scene::Entity& entity : scene.entities())
            {
                pointLightCount += entity.light != nullptr ? 1U : 0U;
            }
            if (ImGui::MenuItem(
                    "Point Light",
                    nullptr,
                    false,
                    pointLightCount < Shader::MaxLights))
            {
                m_selectedEntityId = scene.createPointLight(
                    glm::vec3(1.0f, 0.72f, 0.38f),
                    2.0f,
                    8.0f,
                    spawnTransform,
                    "Point Light");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (m_showRendererConfig)
    {
        ImGui::SetNextWindowSize(ImVec2(360.0f, 420.0f), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Renderer Configuration", &m_showRendererConfig))
        {
            ImGui::BeginDisabled(!m_inputEnabled);
            RendererSettings settings = m_renderer.settings();
            bool settingsChanged = false;

            const char* drawModes[] = {"Solid", "Wireframe", "Points"};
            int drawMode = static_cast<int>(settings.drawMode);
            if (ImGui::Combo("Draw mode", &drawMode, drawModes, 3))
            {
                settings.drawMode = static_cast<RendererDrawMode>(drawMode);
                settingsChanged = true;
            }

            const char* projectionModes[] = {"Perspective", "Orthographic"};
            int projectionMode = camera.projectionMode() == Camera::ProjectionMode::Perspective
                ? 0
                : 1;
            if (ImGui::Combo("Projection", &projectionMode, projectionModes, 2))
            {
                camera.setProjectionMode(
                    projectionMode == 0
                        ? Camera::ProjectionMode::Perspective
                        : Camera::ProjectionMode::Orthographic);
            }
            if (camera.projectionMode() == Camera::ProjectionMode::Perspective)
            {
                float fieldOfView = camera.fieldOfViewDegrees();
                if (ImGui::SliderFloat("Field of view", &fieldOfView, 20.0f, 120.0f, "%.1f deg"))
                {
                    camera.setFieldOfViewDegrees(fieldOfView);
                }
            }
            else
            {
                float orthographicHeight = camera.orthographicHeight();
                if (ImGui::DragFloat(
                        "View height",
                        &orthographicHeight,
                        0.1f,
                        0.1f,
                        200.0f,
                        "%.1f"))
                {
                    camera.setOrthographicHeight(orthographicHeight);
                }
            }
            float nearPlane = camera.nearPlane();
            float farPlane = camera.farPlane();
            bool clipPlanesChanged = false;
            if (ImGui::DragFloat(
                    "Near plane",
                    &nearPlane,
                    0.01f,
                    0.01f,
                    glm::max(farPlane - 0.01f, 0.01f),
                    "%.2f"))
            {
                clipPlanesChanged = true;
            }
            if (ImGui::DragFloat(
                    "Far plane",
                    &farPlane,
                    0.1f,
                    nearPlane + 0.01f,
                    10000.0f,
                    "%.1f"))
            {
                clipPlanesChanged = true;
            }
            if (clipPlanesChanged)
            {
                camera.setClipPlanes(nearPlane, farPlane);
            }

            ImGui::SeparatorText("Debug");
            settingsChanged |= ImGui::Checkbox("Depth testing", &settings.depthTesting);
            settingsChanged |= ImGui::Checkbox("Directional shadows", &settings.shadowsEnabled);
            settingsChanged |= ImGui::Checkbox("Skybox", &settings.skyboxEnabled);
            settingsChanged |= ImGui::Checkbox("Light markers", &settings.lightMarkersEnabled);
            settingsChanged |= ImGui::Checkbox("Light glow", &settings.lightGlowsEnabled);
            settingsChanged |= ImGui::ColorEdit4("Clear color", &settings.clearColor.x);

            if (settingsChanged)
            {
                m_renderer.setSettings(settings);
            }
            ImGui::EndDisabled();
        }
        ImGui::End();
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 workPosition = viewport->WorkPos;
    const ImVec2 workSize = viewport->WorkSize;
    const float viewportRight = viewport->Pos.x + viewport->Size.x;
    constexpr float panelEdgeOverlap = 2.0f;
    const float hierarchyWidth = glm::clamp(workSize.x * 0.19f, 220.0f, 300.0f);
    const float inspectorWidth = glm::clamp(workSize.x * 0.25f, 300.0f, 390.0f);
    const float inspectorLeft = viewportRight - inspectorWidth;
    const ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    const float panelHeight = glm::max(workSize.y - 28.0f, 100.0f);
    if (m_showHierarchy)
    {
        ImGui::SetNextWindowPos(workPosition, ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2(hierarchyWidth, panelHeight),
            ImGuiCond_Always);
        if (ImGui::Begin("Scene Hierarchy", &m_showHierarchy, panelFlags))
        {
            ImGui::BeginDisabled(!m_inputEnabled);
            ImGui::TextUnformatted("HIERARCHY");
            ImGui::SameLine();
            ImGui::TextDisabled("%zu", scene.entities().size());
            ImGui::Separator();
            ImGui::PushID("scene-root");
            const bool sceneOpen = ImGui::TreeNodeEx(
                "Physics Sandbox",
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
            if (ImGui::IsItemClicked())
            {
                m_selectedEntityId = 0;
            }
            if (sceneOpen)
            {
                ImGui::Indent();
                for (Scene::Entity& entity : scene.entities())
                {
                    ImGui::PushID(static_cast<int>(entity.id));
                    const bool isSelected = m_selectedEntityId == entity.id;
                    const ImGuiTreeNodeFlags entityFlags =
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_NoTreePushOnOpen |
                        ImGuiTreeNodeFlags_SpanAvailWidth |
                        (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
                    ImGui::TreeNodeEx(entity.name.c_str(), entityFlags);
                    if (ImGui::IsItemClicked())
                    {
                        m_selectedEntityId = entity.id;
                    }
                    ImGui::PopID();
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::EndDisabled();
        }
        ImGui::End();
    }

    bool deleteSelectedEntity = false;
    if (m_showInspector)
    {
        ImGui::SetNextWindowPos(
            ImVec2(inspectorLeft, workPosition.y),
            ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2(viewportRight - inspectorLeft + panelEdgeOverlap, panelHeight),
            ImGuiCond_Always);
        if (ImGui::Begin("Inspector", &m_showInspector, panelFlags))
        {
            ImGui::BeginDisabled(!m_inputEnabled);
            ImGui::TextUnformatted("INSPECTOR");
            ImGui::Separator();
            if (m_selectedEntityId == 0)
            {
                drawSceneInspector(scene, camera);
            }
            else
            {
                auto& entities = scene.entities();
                const auto found = std::find_if(
                    entities.begin(),
                    entities.end(),
                    [this](const Scene::Entity& entity)
                    {
                        return entity.id == m_selectedEntityId;
                    });
                if (found == entities.end())
                {
                    m_selectedEntityId = 0;
                    ImGui::TextUnformatted("Select an entity from the hierarchy.");
                }
                else
                {
                    drawEntityInspector(*found);
                    ImGui::Separator();
                    deleteSelectedEntity = ImGui::Button("Delete Entity");
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::End();
    }

    if (deleteSelectedEntity)
    {
        scene.removeEntity(m_selectedEntityId);
        m_selectedEntityId = 0;
    }

    ImGui::SetNextWindowPos(
        ImVec2(workPosition.x, workPosition.y + workSize.y - 28.0f),
        ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(workSize.x, 28.0f), ImGuiCond_Always);
    if (ImGui::Begin(
            "Editor Status",
            nullptr,
            ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text(
            "FPS %.1f  |  %.2f ms  |  %zu entities  |  %s controls",
            framesPerSecond,
            frameMilliseconds,
            scene.entities().size(),
            m_inputEnabled ? "Editor" : "Camera");
    }
    ImGui::End();
}

void Gui::endFrame()
{
    if (m_destroyed || !m_frameStarted)
    {
        throw std::logic_error("No GUI frame is active.");
    }

    ImGui::Render();

    glDisable(GL_FRAMEBUFFER_SRGB);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glEnable(GL_FRAMEBUFFER_SRGB);

    m_frameStarted = false;
}

void Gui::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    if (m_frameStarted)
    {
        ImGui::EndFrame();
        m_frameStarted = false;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_destroyed = true;
}
