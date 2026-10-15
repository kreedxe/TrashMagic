#pragma once

#include <functional>

class Window;

enum class InputMode
{
    Gui,
    Camera
};

class InputEventSystem
{
public:
    explicit InputEventSystem(Window& window);
    ~InputEventSystem();

    InputEventSystem(const InputEventSystem&) = delete;
    InputEventSystem& operator=(const InputEventSystem&) = delete;

    InputMode mode() const;
    bool cameraControlsActive() const;
    void setModeChangedCallback(std::function<void(InputMode)> callback);
    void destroy();

private:
    void handleKey(int key, int action);
    void setMode(InputMode mode);

    Window& m_window;
    InputMode m_mode = InputMode::Gui;
    std::function<void(InputMode)> m_modeChangedCallback;
    bool m_destroyed = false;
};
