#include "Prefix.h"

#include "GLFWInputDevice.h"
#include "GLFWInputSystem.h"
#include "Input/InputSystem.h"
#include "Input/InputAction.h"
#include "Windowing/Window.h"

#include <GLFW/glfw3.h>

namespace gore
{

static int GetGLFWKeyCode(KeyCode keyCode);

GLFWKeyboard::GLFWKeyboard(Window* window) :
    Keyboard(),
    m_Window(window)
{
}

GLFWKeyboard::~GLFWKeyboard()
{
}

void GLFWKeyboard::Update()
{
    for (int i = 0; i < static_cast<int>(KeyCode::Count); ++i)
    {
        const KeyCode keyCode = static_cast<KeyCode>(i);
        const int glfwKeyCode = GetGLFWKeyCode(keyCode);

        const bool state           = glfwGetKey(m_Window->Get(), glfwKeyCode) == GLFW_PRESS;
        DigitalState& digitalState = m_Keys[i];

        digitalState.lastState = digitalState.state;
        digitalState.state     = state;
    }

    UpdateAllActions();
}

GLFWMouse::GLFWMouse(Window* window) :
    Mouse(),
    m_Window(window),
    m_ScrollX(0.0),
    m_ScrollY(0.0)
{
    glfwSetScrollCallback(m_Window->Get(), [](GLFWwindow* window, double xoffset, double yoffset)
                          {
                              const GLFWInputSystem* inputSystem = reinterpret_cast<const GLFWInputSystem*>(InputSystem::Get());
                              GLFWMouse* mouse = reinterpret_cast<GLFWMouse*>(inputSystem->GetMouse());
                              mouse->ScrollCallback(window, xoffset, yoffset);
                          });
}

GLFWMouse::~GLFWMouse()
{
}

void GLFWMouse::Update()
{
    for (int i = 0; i < static_cast<int>(MouseButtonCode::Count); ++i)
    {
        const MouseButtonCode buttonCode = static_cast<MouseButtonCode>(i);

        const bool state           = glfwGetMouseButton(m_Window->Get(), i) == GLFW_PRESS;
        DigitalState& digitalState = m_Buttons[i];

        digitalState.lastState = digitalState.state;
        digitalState.state     = state;
    }

    double x, y;
    glfwGetCursorPos(m_Window->Get(), &x, &y);

    m_Movements[static_cast<int>(MouseMovementCode::X)].lastState = m_Movements[static_cast<int>(MouseMovementCode::X)].state;
    m_Movements[static_cast<int>(MouseMovementCode::Y)].lastState = m_Movements[static_cast<int>(MouseMovementCode::Y)].state;
    m_Movements[static_cast<int>(MouseMovementCode::X)].state     = static_cast<float>(x);
    m_Movements[static_cast<int>(MouseMovementCode::Y)].state     = static_cast<float>(y);

    m_Movements[static_cast<int>(MouseMovementCode::ScrollX)].lastState = m_Movements[static_cast<int>(MouseMovementCode::ScrollX)].state;
    m_Movements[static_cast<int>(MouseMovementCode::ScrollY)].lastState = m_Movements[static_cast<int>(MouseMovementCode::ScrollY)].state;
    m_Movements[static_cast<int>(MouseMovementCode::ScrollX)].state     = static_cast<float>(m_ScrollX);
    m_Movements[static_cast<int>(MouseMovementCode::ScrollY)].state     = static_cast<float>(m_ScrollY);

    UpdateAllActions();
}

void GLFWMouse::SetCursorShow(bool show) const
{
    glfwSetInputMode(m_Window->Get(), GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void GLFWMouse::ScrollCallback(GLFWwindow* window, double x, double y)
{
    m_ScrollX += x;
    m_ScrollY += y;
}

int GetGLFWKeyCode(KeyCode keyCode)
{
    switch (keyCode)
    {
        case KeyCode::Backspace:
            return GLFW_KEY_BACKSPACE;
        case KeyCode::Delete:
            return GLFW_KEY_DELETE;
        case KeyCode::Tab:
            return GLFW_KEY_TAB;
        case KeyCode::Return:
            return GLFW_KEY_ENTER;
        case KeyCode::Esc:
            return GLFW_KEY_ESCAPE;
        case KeyCode::Space:
            return GLFW_KEY_SPACE;
        case KeyCode::Keypad0:
            return GLFW_KEY_KP_0;
        case KeyCode::Keypad1:
            return GLFW_KEY_KP_1;
        case KeyCode::Keypad2:
            return GLFW_KEY_KP_2;
        case KeyCode::Keypad3:
            return GLFW_KEY_KP_3;
        case KeyCode::Keypad4:
            return GLFW_KEY_KP_4;
        case KeyCode::Keypad5:
            return GLFW_KEY_KP_5;
        case KeyCode::Keypad6:
            return GLFW_KEY_KP_6;
        case KeyCode::Keypad7:
            return GLFW_KEY_KP_7;
        case KeyCode::Keypad8:
            return GLFW_KEY_KP_8;
        case KeyCode::Keypad9:
            return GLFW_KEY_KP_9;
        case KeyCode::KeypadPeriod:
            return GLFW_KEY_KP_DECIMAL;
        case KeyCode::KeypadDivide:
            return GLFW_KEY_KP_DIVIDE;
        case KeyCode::KeypadMultiply:
            return GLFW_KEY_KP_MULTIPLY;
        case KeyCode::KeypadMinus:
            return GLFW_KEY_KP_SUBTRACT;
        case KeyCode::KeypadPlus:
            return GLFW_KEY_KP_ADD;
        case KeyCode::KeypadEnter:
            return GLFW_KEY_KP_ENTER;
        case KeyCode::Up:
            return GLFW_KEY_UP;
        case KeyCode::Down:
            return GLFW_KEY_DOWN;
        case KeyCode::Right:
            return GLFW_KEY_RIGHT;
        case KeyCode::Left:
            return GLFW_KEY_LEFT;
        case KeyCode::Insert:
            return GLFW_KEY_INSERT;
        case KeyCode::Home:
            return GLFW_KEY_HOME;
        case KeyCode::End:
            return GLFW_KEY_END;
        case KeyCode::PageUp:
            return GLFW_KEY_PAGE_UP;
        case KeyCode::PageDown:
            return GLFW_KEY_PAGE_DOWN;
        case KeyCode::F1:
            return GLFW_KEY_F1;
        case KeyCode::F2:
            return GLFW_KEY_F2;
        case KeyCode::F3:
            return GLFW_KEY_F3;
        case KeyCode::F4:
            return GLFW_KEY_F4;
        case KeyCode::F5:
            return GLFW_KEY_F5;
        case KeyCode::F6:
            return GLFW_KEY_F6;
        case KeyCode::F7:
            return GLFW_KEY_F7;
        case KeyCode::F8:
            return GLFW_KEY_F8;
        case KeyCode::F9:
            return GLFW_KEY_F9;
        case KeyCode::F10:
            return GLFW_KEY_F10;
        case KeyCode::F11:
            return GLFW_KEY_F11;
        case KeyCode::F12:
            return GLFW_KEY_F12;
        case KeyCode::F13:
            return GLFW_KEY_F13;
        case KeyCode::F14:
            return GLFW_KEY_F14;
        case KeyCode::F15:
            return GLFW_KEY_F15;
        case KeyCode::Number0:
            return GLFW_KEY_0;
        case KeyCode::Number1:
            return GLFW_KEY_1;
        case KeyCode::Number2:
            return GLFW_KEY_2;
        case KeyCode::Number3:
            return GLFW_KEY_3;
        case KeyCode::Number4:
            return GLFW_KEY_4;
        case KeyCode::Number5:
            return GLFW_KEY_5;
        case KeyCode::Number6:
            return GLFW_KEY_6;
        case KeyCode::Number7:
            return GLFW_KEY_7;
        case KeyCode::Number8:
            return GLFW_KEY_8;
        case KeyCode::Number9:
            return GLFW_KEY_9;
        case KeyCode::Quote:
            return GLFW_KEY_APOSTROPHE;
        case KeyCode::Comma:
            return GLFW_KEY_COMMA;
        case KeyCode::Minus:
            return GLFW_KEY_MINUS;
        case KeyCode::Period:
            return GLFW_KEY_PERIOD;
        case KeyCode::Slash:
            return GLFW_KEY_SLASH;
        case KeyCode::Semicolon:
            return GLFW_KEY_SEMICOLON;
        case KeyCode::Equal:
            return GLFW_KEY_EQUAL;
        case KeyCode::LeftBracket:
            return GLFW_KEY_LEFT_BRACKET;
        case KeyCode::Backslash:
            return GLFW_KEY_BACKSLASH;
        case KeyCode::RightBracket:
            return GLFW_KEY_RIGHT_BRACKET;
        case KeyCode::BackQuote:
            return GLFW_KEY_GRAVE_ACCENT;
        case KeyCode::A:
            return GLFW_KEY_A;
        case KeyCode::B:
            return GLFW_KEY_B;
        case KeyCode::C:
            return GLFW_KEY_C;
        case KeyCode::D:
            return GLFW_KEY_D;
        case KeyCode::E:
            return GLFW_KEY_E;
        case KeyCode::F:
            return GLFW_KEY_F;
        case KeyCode::G:
            return GLFW_KEY_G;
        case KeyCode::H:
            return GLFW_KEY_H;
        case KeyCode::I:
            return GLFW_KEY_I;
        case KeyCode::J:
            return GLFW_KEY_J;
        case KeyCode::K:
            return GLFW_KEY_K;
        case KeyCode::L:
            return GLFW_KEY_L;
        case KeyCode::M:
            return GLFW_KEY_M;
        case KeyCode::N:
            return GLFW_KEY_N;
        case KeyCode::O:
            return GLFW_KEY_O;
        case KeyCode::P:
            return GLFW_KEY_P;
        case KeyCode::Q:
            return GLFW_KEY_Q;
        case KeyCode::R:
            return GLFW_KEY_R;
        case KeyCode::S:
            return GLFW_KEY_S;
        case KeyCode::T:
            return GLFW_KEY_T;
        case KeyCode::U:
            return GLFW_KEY_U;
        case KeyCode::V:
            return GLFW_KEY_V;
        case KeyCode::W:
            return GLFW_KEY_W;
        case KeyCode::X:
            return GLFW_KEY_X;
        case KeyCode::Y:
            return GLFW_KEY_Y;
        case KeyCode::Z:
            return GLFW_KEY_Z;
        case KeyCode::NumLock:
            return GLFW_KEY_NUM_LOCK;
        case KeyCode::CapsLock:
            return GLFW_KEY_CAPS_LOCK;
        case KeyCode::ScrollLock:
            return GLFW_KEY_SCROLL_LOCK;
        case KeyCode::RightShift:
            return GLFW_KEY_RIGHT_SHIFT;
        case KeyCode::LeftShift:
            return GLFW_KEY_LEFT_SHIFT;
        case KeyCode::RightControl:
            return GLFW_KEY_RIGHT_CONTROL;
        case KeyCode::LeftControl:
            return GLFW_KEY_LEFT_CONTROL;
        case KeyCode::RightAlt:
            return GLFW_KEY_RIGHT_ALT;
        case KeyCode::LeftAlt:
            return GLFW_KEY_LEFT_ALT;
        case KeyCode::LeftWindows:
            return GLFW_KEY_LEFT_SUPER;
        case KeyCode::RightWindows:
            return GLFW_KEY_RIGHT_SUPER;
        case KeyCode::Menu:
            return GLFW_KEY_MENU;
        case KeyCode::Unknown:
        default:
            return GLFW_KEY_UNKNOWN;
    }
}

} // namespace gore