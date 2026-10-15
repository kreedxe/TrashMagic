#include "InputEventSystem.hpp"

#include "Window.hpp"

#include <GLFW/glfw3.h>

#include <utility>

InputEventSystem::InputEventSystem(Window& window)
    : m_window(window)
{
    m_window.setKeyCallback([this](int key, int, int action, int)
    {
        handleKey(key, action);
    });
    m_window.setCursorCaptured(false);
}

InputEventSystem::~InputEventSystem()
{
    destroy();
}

InputMode InputEventSystem::mode() const
{
    return m_mode;
}

bool InputEventSystem::cameraControlsActive() const
{
    return !m_destroyed && m_mode == InputMode::Camera;
}

void InputEventSystem::setModeChangedCallback(std::function<void(InputMode)> callback)
{
    m_modeChangedCallback = std::move(callback);
    if (m_modeChangedCallback)
    {
        m_modeChangedCallback(m_mode);
    }
}

void InputEventSystem::handleKey(int key, int action)
{
    if (m_destroyed || action != GLFW_PRESS)
    {
        return;
    }

    if (key == GLFW_KEY_ESCAPE)
    {
        m_window.requestClose();
    }
    else if (key == GLFW_KEY_TAB)
    {
        setMode(m_mode == InputMode::Gui ? InputMode::Camera : InputMode::Gui);
    }
}

void InputEventSystem::setMode(InputMode mode)
{
    if (m_mode == mode)
    {
        return;
    }

    m_mode = mode;
    m_window.setCursorCaptured(mode == InputMode::Camera);
    if (m_modeChangedCallback)
    {
        m_modeChangedCallback(m_mode);
    }
}

void InputEventSystem::destroy()
{
    if (m_destroyed)
    {
        return;
    }
    m_window.setKeyCallback({});
    m_modeChangedCallback = {};
    m_destroyed = true;
}
