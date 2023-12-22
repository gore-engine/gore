#include "Prefix.h"

#include "App.h"

#include <GLFW/glfw3.h>

#include <algorithm>

#include "Core/Time.h"

namespace gore
{

App::App(int argc, char** argv) :
    m_Args(argv + 1, argv + argc),
    m_Window(nullptr),
    m_TimeSystem(nullptr)
{
}

App::~App()
{
}

void App::MainLoop()
{
    Update();

    Render();
}

bool App::HasArg(const std::string& arg) const
{
    std::string argWithDash = "-" + arg;
    return std::find(m_Args.begin(), m_Args.end(), argWithDash) != m_Args.end();
}

int App::Run(uint32_t width, uint32_t height, const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#if PLATFORM_MACOS
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(primaryMonitor, &xscale, &yscale);
#else
    float xscale = 1.0f;
    float yscale = 1.0f;
#endif

    m_Window = glfwCreateWindow((int)((float)width / xscale), (int)((float)height / yscale), title, NULL, NULL);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, OnWindowResizeCallback);
    InitNativeWindowHandle();

    Initialize();

    m_TimeSystem = new Time(this);
    m_TimeSystem->Initialize();

    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();

        m_TimeSystem->Update();

        MainLoop();
    }

    m_TimeSystem->Shutdown();

    Shutdown();

    DestroyNativeWindowHandle();
    glfwDestroyWindow(m_Window);
    glfwTerminate();

    return 0;
}

void App::SetWindowTitle(const std::string& title)
{
    glfwSetWindowTitle(m_Window, title.c_str());
}

void App::GetWindowSize(int* width, int* height) const
{
    glfwGetFramebufferSize(m_Window, width, height);
}


void App::OnWindowResizeCallback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->OnWindowResize(width, height);
}

} // namespace gore