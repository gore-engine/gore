#pragma once

#include "Input/InputDevice.h"

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

    void SetCursorShow(bool show) final;

private:
    Window* m_Window;
};

} // namespace gore
