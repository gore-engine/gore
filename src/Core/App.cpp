#include "Prefix.h"

#include "App.h"

#include <GLFW/glfw3.h>

#include <algorithm>

namespace gore
{

App::App(int argc, char** argv) :
    m_Args(argv + 1, argv + argc),
    m_Window(nullptr)
{
    m_LastTime  = std::chrono::steady_clock::now();
    m_StartTime = m_LastTime;
}

App::~App()
{
}

void App::MainLoop()
{
    auto currentTime = std::chrono::steady_clock::now();
    // deltaTime in seconds
    float deltaTime = std::chrono::duration<float>(currentTime - m_LastTime).count();
    float totalTime = std::chrono::duration<float>(currentTime - m_StartTime).count();
    m_LastTime      = currentTime;

    Update(deltaTime, totalTime);

    Render(deltaTime, totalTime);
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
    m_LastTime  = std::chrono::steady_clock::now();
    m_StartTime = m_LastTime;

    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
        MainLoop();
    }

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