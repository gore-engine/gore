#pragma once

#include "Export.h"

#include "KeyCode.h"

namespace gore
{

ENGINE_CLASS(InputDevice)
{
public:
    InputDevice() = default;
    virtual ~InputDevice();

    NON_COPYABLE(InputDevice);

    virtual void Update() = 0;

protected:
    struct DigitalState
    {
        bool state;
        bool lastState;
    };

    struct AnalogState
    {
        float state;
        float lastState;
    };
};

ENGINE_CLASS(Keyboard) : public InputDevice
{
public:
    Keyboard();
    virtual ~Keyboard();

    NON_COPYABLE(Keyboard);

    [[nodiscard]] bool KeyState(KeyCode key) const;
    [[nodiscard]] bool KeyPressed(KeyCode key) const;
    [[nodiscard]] bool KeyReleased(KeyCode key) const;

protected:
    DigitalState m_Keys[static_cast<int>(KeyCode::Count)];
};

ENGINE_CLASS(Mouse) : public InputDevice
{
public:
    Mouse();
    virtual ~Mouse();

    NON_COPYABLE(Mouse);

    [[nodiscard]] bool ButtonState(MouseButtonCode button) const;
    [[nodiscard]] bool ButtonPressed(MouseButtonCode button) const;
    [[nodiscard]] bool ButtonReleased(MouseButtonCode button) const;

    [[nodiscard]] float Get(MouseMovementCode movement) const;
    [[nodiscard]] float GetDelta(MouseMovementCode movement) const;

    virtual void SetCursorShow(bool show) = 0;

protected:
    DigitalState m_Buttons[static_cast<int>(MouseButtonCode::Count)];
    AnalogState m_Movements[static_cast<int>(MouseMovementCode::Count)];
};

}
