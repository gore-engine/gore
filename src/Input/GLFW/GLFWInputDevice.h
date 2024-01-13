#pragma once

#include "Input/InputDevice.h"

typedef struct GLFWwindow GLFWwindow;

namespace gore
{

class Window;

class GLFWKeyboard final : public Keyboard
{
public:
    explicit GLFWKeyboard(Window* window);
    ~GLFWKeyboard() final;

    void Update() final;

private:
    Window* m_Window;
};

class GLFWMouse final : public Mouse
{
public:
    explicit GLFWMouse(Window* window);
    ~GLFWMouse() final;

    void Update() final;

    void SetCursorShow(bool show) const final;

private:
    Window* m_Window;

    double m_ScrollX;
    double m_ScrollY;

    void ScrollCallback(GLFWwindow* window, double x, double y);
};

} // namespace gore
