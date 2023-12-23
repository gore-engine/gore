#include "Prefix.h"

#include "Window.h"

#include "Core/App.h"

#include <GLFW/glfw3.h>

namespace gore
{

Window::Window(App* app, int width, int height) :
    m_App(app),
    m_Window(nullptr),
    m_NativeHandle(nullptr)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#if PLATFORM_MACOS
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primaryMonitor, &xscale, &yscale);
#else
    float xscale = 1.0f;
    float yscale = 1.0f;
#endif

    m_Window = glfwCreateWindow((int)((float)width / xscale), (int)((float)height / yscale), "gore", NULL, NULL);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, OnWindowResizeCallback);

    CreateNativeHandle();
}

Window::~Window()
{
    DestroyNativeHandle();
    glfwDestroyWindow(m_Window);
}

void Window::GetSize(int* width, int* height) const
{
    glfwGetFramebufferSize(m_Window, width, height);
}

bool Window::ShouldClose() const
{
    bool result = glfwWindowShouldClose(m_Window);
    if (!result)
        glfwPollEvents();
    return result;
}

void Window::SetTitle(const std::string& title)
{
    glfwSetWindowTitle(m_Window, title.c_str());
}

void Window::OnWindowResizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->m_App->OnWindowResize(window, width, height);
}

} // namespace gore