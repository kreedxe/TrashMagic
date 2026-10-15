#include "Window.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{
std::unordered_map<GLFWwindow*, Window*>& windowInstances()
{
    static std::unordered_map<GLFWwindow*, Window*> instances;
    return instances;
}
}

void Window::setKeyCallback(std::function<void(int, int, int, int)> callback)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot set a key callback on a destroyed window.");
    }
    m_keyCallback = std::move(callback);
}

Window::Window(int width, int height, const std::string& title)
    : m_framebufferWidth(width),
      m_framebufferHeight(height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("Window dimensions must be greater than zero.");
    }

    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_window == nullptr)
    {
        const char* description = nullptr;
        glfwGetError(&description);
        const std::string error = description != nullptr
            ? description
            : "Failed to create a GLFW window.";
        glfwTerminate();
        throw std::runtime_error(error);
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    windowInstances().emplace(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetKeyCallback(m_window, keyCallback);

    glfwGetFramebufferSize(m_window, &m_framebufferWidth, &m_framebufferHeight);
}

Window::~Window()
{
    destroy();
}

void Window::destroy()
{
    if (m_window != nullptr)
    {
        glfwSetFramebufferSizeCallback(m_window, nullptr);
        glfwSetScrollCallback(m_window, nullptr);
        glfwSetKeyCallback(m_window, nullptr);
        windowInstances().erase(m_window);
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
    }
    m_framebufferSizeCallback = {};
    m_keyCallback = {};
    m_destroyed = true;
}

void Window::pollEvents() const
{
    if (!m_destroyed)
    {
        glfwPollEvents();
    }
}

void Window::swapBuffers() const
{
    if (!m_destroyed)
    {
        glfwSwapBuffers(m_window);
    }
}

void Window::setVSync(bool enabled)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot change VSync on a destroyed window.");
    }
    glfwSwapInterval(enabled ? 1 : 0);
}

bool Window::isKeyDown(int key) const
{
    return !m_destroyed && glfwGetKey(m_window, key) == GLFW_PRESS;
}

void Window::setCursorCaptured(bool captured)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot change cursor mode on a destroyed window.");
    }
    glfwSetInputMode(m_window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Window::getCursorPosition(double& x, double& y) const
{
    if (m_destroyed)
    {
        x = 0.0;
        y = 0.0;
        return;
    }
    glfwGetCursorPos(m_window, &x, &y);
}

double Window::consumeScrollDelta() const
{
    const double delta = m_scrollDelta;
    m_scrollDelta = 0.0;
    return delta;
}

bool Window::shouldClose() const
{
    return m_destroyed || glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::requestClose() const
{
    if (!m_destroyed)
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

int Window::getFramebufferWidth() const
{
    return m_framebufferWidth;
}

int Window::getFramebufferHeight() const
{
    return m_framebufferHeight;
}

GLFWwindow* Window::getNativeHandle() const
{
    return m_window;
}

void Window::setFramebufferSizeCallback(std::function<void(int, int)> callback)
{
    if (m_destroyed)
    {
        throw std::logic_error("Cannot set a callback on a destroyed window.");
    }
    m_framebufferSizeCallback = std::move(callback);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    const auto found = windowInstances().find(window);
    if (found == windowInstances().end())
    {
        return;
    }
    Window* instance = found->second;

    instance->m_framebufferWidth = width;
    instance->m_framebufferHeight = height;
    if (instance->m_framebufferSizeCallback)
    {
        instance->m_framebufferSizeCallback(width, height);
    }
}

void Window::scrollCallback(GLFWwindow* window, double, double yOffset)
{
    const auto found = windowInstances().find(window);
    if (found != windowInstances().end())
    {
        found->second->m_scrollDelta += yOffset;
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const auto found = windowInstances().find(window);
    if (found != windowInstances().end() && found->second->m_keyCallback)
    {
        found->second->m_keyCallback(key, scancode, action, mods);
    }
}
