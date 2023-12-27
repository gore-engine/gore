#include "Prefix.h"

#include "App.h"

#include <GLFW/glfw3.h>

#include <algorithm>

#include "Core/Time.h"
#include "Rendering/RenderSystem.h"
#include "Windowing/Window.h"
#include "Scene/Scene.h"

namespace gore
{

App::App(int argc, char** argv) :
    m_Args(argv + 1, argv + argc),
    m_TimeSystem(nullptr),
    m_RenderSystem(nullptr),
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
    m_RenderSystem->OnResize(window, width, height);
}

int App::Run(int width, int height, const char* title)
{
    glfwInit();

    m_Window = new Window(this, width, height);
    m_Window->SetTitle(title);

    m_RenderSystem = new RenderSystem(this);
    m_RenderSystem->Initialize();

    Initialize();

    m_TimeSystem = new Time(this);
    m_TimeSystem->Initialize();

    while (!m_Window->ShouldClose())
    {
        m_TimeSystem->Update();

        Update();

        std::vector<Scene*> scenes = Scene::GetScenes();
        for (Scene* scene : scenes)
            scene->Update();

        m_RenderSystem->Update();
    }

    m_TimeSystem->Shutdown();

    Shutdown();

    m_RenderSystem->Shutdown();


    delete m_TimeSystem;
    delete m_RenderSystem;

    delete m_Window;

    glfwTerminate();

    return 0;
}

} // namespace gore