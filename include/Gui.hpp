#pragma once

#include <imgui.h>

class Camera;
class Renderer;
class Scene;
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
    void drawDebugOverlay(float framesPerSecond, float frameMilliseconds, const Camera& camera, Scene& scene);
    void endFrame();
    void destroy();

private:
    Window& m_window;
    Renderer& m_renderer;
    bool m_frameStarted = false;
    bool m_destroyed = false;
    bool m_inputEnabled = true;
};
