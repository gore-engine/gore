#include "Prefix.h"

#include "App.h"

#include <GLFW/glfw3.h>

#include <algorithm>

#include "Core/Time.h"
#include "Rendering/RenderSystem.h"
#include "Windowing/Window.h"
#include "Scene/Scene.h"
#include "Input/GLFW/GLFWInputSystem.h"

#include "Profiler/microprofile.h"

MICROPROFILE_DEFINE(g_PlayerLoop, "Loop", "PlayerLoop", MP_AUTO);
MICROPROFILE_DEFINE(g_AppUpdate, "Loop", "AppUpdate", MP_AUTO);
MICROPROFILE_DEFINE(g_SceneUpdate, "Loop", "SceneUpdate", MP_AUTO);
MICROPROFILE_DEFINE(g_RenderSystemUpdate, "Loop", "RenderSystemUpdate", MP_AUTO);
MICROPROFILE_DEFINE(g_AppInitialize, "System", "AppInitialize", MP_AUTO);

namespace gore
{

static App *g_App = nullptr;

App::App(int argc, char** argv) :
    m_Args(argv + 1, argv + argc),
    m_ExecutablePath(argv[0]),
    m_TimeSystem(nullptr),
    m_InputSystem(nullptr),
    m_RenderSystem(nullptr),
    m_Window(nullptr)
{
    g_App = this;
}

App::~App()
{
    g_App = nullptr;
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
    MicroProfileOnThreadCreate("Main");
    MicroProfileSetEnableAllGroups(true);
    MicroProfileSetForceMetaCounters(true);

    MICROPROFILE_SCOPE(g_AppInitialize);
    {   
        glfwInit();
        
        m_Window = new Window(this, width, height);
        m_Window->SetTitle(title);
        
        // TODO: Choose backend
        m_InputSystem = new GLFWInputSystem(this);
        m_InputSystem->Initialize();
        
        m_RenderSystem = new RenderSystem(this);
        m_RenderSystem->Initialize();
        
        Initialize();
        
        m_TimeSystem = new Time(this);
        m_TimeSystem->Initialize();
    }

    while (!m_Window->ShouldClose())
    {
        {
            MICROPROFILE_SCOPE(g_PlayerLoop);
            m_TimeSystem->Update();
            m_InputSystem->Update();
        }
        
        {
            MICROPROFILE_SCOPE(g_AppUpdate);
            Update();
        }

        {
            MICROPROFILE_SCOPE(g_SceneUpdate);
            std::vector<Scene*> scenes = Scene::GetScenes();
            for (Scene* scene : scenes)
            scene->Update();
        }

        {
            MICROPROFILE_SCOPE(g_RenderSystemUpdate);
            m_RenderSystem->Update();
        }

        EndofFrame();

        MicroProfileFlip(nullptr);
    }

    m_TimeSystem->Shutdown();

    Shutdown();

    m_RenderSystem->Shutdown();
    m_InputSystem->Shutdown();

    delete m_TimeSystem;
    delete m_RenderSystem;
    delete m_InputSystem;

    delete m_Window;

    glfwTerminate();

    MicroProfileShutdown();

    return 0;
}

App* App::Get()
{
    return g_App;
}

} // namespace gore
