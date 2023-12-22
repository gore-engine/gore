#include "Prefix.h"

#include "App.h"

#include <GLFW/glfw3.h>

#include <algorithm>

#include "Core/Time.h"
#include "Windowing/Window.h"

namespace gore
{

App::App(int argc, char** argv) :
    m_Args(argv + 1, argv + argc),
    m_TimeSystem(nullptr),
    m_Window(nullptr)
{
}

App::~App()
{
}

bool App::HasArg(const std::string& arg) const
{
    std::string argWithDash = "-" + arg;
    return std::find(m_Args.begin(), m_Args.end(), argWithDash) != m_Args.end();
}

void App::OnWindowResize(Window* window, int width, int height)
{
}

int App::Run(int width, int height, const char* title)
{
    glfwInit();

    m_Window = new Window(this, width, height);
    m_Window->SetTitle(title);

    Initialize();

    m_TimeSystem = new Time(this);
    m_TimeSystem->Initialize();

    while (!m_Window->ShouldClose())
    {
        m_TimeSystem->Update();

        Update();

        Render();
    }

    m_TimeSystem->Shutdown();

    Shutdown();

    delete m_TimeSystem;

    delete m_Window;

    glfwTerminate();

    return 0;
}

} // namespace gore