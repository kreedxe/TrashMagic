#include "Gui.hpp"

#include "Camera.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <glm/geometric.hpp>

#include <stdexcept>

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
    ImGui::StyleColorsDark();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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

void Gui::drawDebugOverlay(
    float framesPerSecond,
    float frameMilliseconds,
    const Camera& camera,
    Scene& scene)
{
    if (m_destroyed || !m_frameStarted)
    {
        throw std::logic_error("Begin the GUI frame before drawing the debug overlay.");
    }

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.75f);
    if (ImGui::Begin("Renderer Debug", nullptr, flags))
    {
        ImGui::BeginDisabled(!m_inputEnabled);
        const glm::vec3& position = camera.position();
        ImGui::Text("FPS: %.1f", framesPerSecond);
        ImGui::Text("Frame: %.2f ms", frameMilliseconds);
        ImGui::Separator();
        ImGui::Text("Camera: %.2f, %.2f, %.2f", position.x, position.y, position.z);
        ImGui::Text("FOV: %.1f deg", camera.fieldOfViewDegrees());
        ImGui::Text("Scene entities: %zu", scene.entities().size());
        ImGui::TextUnformatted("Tab: switch controls | Escape: quit");
        ImGui::EndDisabled();
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(12.0f, 200.0f), ImGuiCond_Always);
    if (ImGui::Begin("Current Scene"))
    {
        ImGui::BeginDisabled(!m_inputEnabled);
        SceneLighting& lighting = scene.lighting();
        ImGui::SliderFloat("Cubemap brightness", &lighting.cubemapBrightness, 0.0f, 3.0f);
        ImGui::ColorEdit3("Ambient light", &lighting.ambientColor.x);
        ImGui::DragFloat3("Sun direction", &lighting.sunDirection.x, 0.02f, -1.0f, 1.0f);
        if (glm::length(lighting.sunDirection) > 0.0001f)
        {
            lighting.sunDirection = glm::normalize(lighting.sunDirection);
        }
        else
        {
            lighting.sunDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        }
        ImGui::ColorEdit3("Sun color", &lighting.sunColor.x);
        ImGui::SliderFloat("Sun intensity", &lighting.sunIntensity, 0.0f, 3.0f);
        ImGui::Text("Cubemap: %s", scene.cubemap() != nullptr ? "assigned" : "none");
        ImGui::EndDisabled();
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
