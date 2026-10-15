#pragma once

#include <imgui.h>

class Camera;
class Cube;
class Pyramid;
class Renderer;
class Scene;
class Shader;
class Sphere;
class Window;

class Gui
{
public:
    Gui(Window& window, Renderer& renderer);
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    void beginFrame();
    void setInputEnabled(bool enabled);
    void drawEditor(
        float framesPerSecond,
        float frameMilliseconds,
        Camera& camera,
        Scene& scene,
        Cube& cubeTemplate,
        Sphere& sphereTemplate,
        Pyramid& pyramidTemplate,
        Shader& shader);
    void endFrame();
    void destroy();

private:
    Window& m_window;
    Renderer& m_renderer;
    unsigned int m_selectedEntityId = 0;
    bool m_frameStarted = false;
    bool m_destroyed = false;
    bool m_inputEnabled = true;
    bool m_showHierarchy = true;
    bool m_showInspector = true;
    bool m_showRendererConfig = false;
};
