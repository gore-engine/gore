#include "Prefix.h"

#include "GLFWInputSystem.h"
#include "GLFWInputDevice.h"

#include "Core/App.h"
#include "Windowing/Window.h"

#include <GLFW/glfw3.h>

namespace gore
{

GLFWInputSystem::GLFWInputSystem(App* app) :
    InputSystem(app),
    m_Keyboard(nullptr),
    m_Mouse(nullptr)
{
}

GLFWInputSystem::~GLFWInputSystem()
{
}

Keyboard* GLFWInputSystem::GetKeyboard() const
{
    return m_Keyboard;
}

Mouse* GLFWInputSystem::GetMouse() const
{
    return m_Mouse;
}

void GLFWInputSystem::Initialize()
{
    m_Keyboard = new GLFWKeyboard(m_App->GetWindow());
    m_Mouse = new GLFWMouse(m_App->GetWindow());

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_App->GetWindow()->Get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void GLFWInputSystem::Update()
{
    m_Keyboard->Update();
    m_Mouse->Update();
}

void GLFWInputSystem::Shutdown()
{
    delete m_Keyboard;
    delete m_Mouse;
}

} // namespace gore