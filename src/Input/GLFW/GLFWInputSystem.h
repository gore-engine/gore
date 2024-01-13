#pragma once

#include "Input/InputSystem.h"

namespace gore
{

class GLFWKeyboard;
class GLFWMouse;

class GLFWInputSystem final : public InputSystem
{
public:
    explicit GLFWInputSystem(App* app);
    ~GLFWInputSystem() final;

    NON_COPYABLE(GLFWInputSystem);

    [[nodiscard]] Keyboard* GetKeyboard() const final;
    [[nodiscard]] Mouse* GetMouse() const final;

    void Initialize() final;
    void Update() final;
    void Shutdown() final;

private:
    GLFWKeyboard* m_Keyboard;
    GLFWMouse* m_Mouse;
};

} // namespace gore
