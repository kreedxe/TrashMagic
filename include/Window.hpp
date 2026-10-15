#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

class Window
{
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void destroy();
    void pollEvents() const;
    void swapBuffers() const;
    void setVSync(bool enabled);
    bool isKeyDown(int key) const;
    void setCursorCaptured(bool captured);
    void getCursorPosition(double& x, double& y) const;
    double consumeScrollDelta() const;
    void setKeyCallback(std::function<void(int, int, int, int)> callback);
    bool shouldClose() const;
    void requestClose() const;

    int getFramebufferWidth() const;
    int getFramebufferHeight() const;
    GLFWwindow* getNativeHandle() const;

    void setFramebufferSizeCallback(std::function<void(int, int)> callback);

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    GLFWwindow* m_window = nullptr;
    int m_framebufferWidth = 0;
    int m_framebufferHeight = 0;
    std::function<void(int, int)> m_framebufferSizeCallback;
    std::function<void(int, int, int, int)> m_keyCallback;
    mutable double m_scrollDelta = 0.0;
    bool m_destroyed = false;
};
