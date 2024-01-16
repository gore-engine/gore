#pragma once

#include "Export.h"

#include "InputEnums.h"

#include <map>
#include <string>
#include <functional>

namespace gore
{

class InputAction;

ENGINE_CLASS(InputDevice)
{
public:
    InputDevice();
    virtual ~InputDevice();

    NON_COPYABLE(InputDevice);

    virtual void Update() = 0;

    [[nodiscard]] const InputAction* GetAction(const std::string& name) const;

protected:
    friend class InputAction;

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

    std::map<std::string, InputAction> m_Actions;

    InputAction* AddAction(const std::string& name, InputType type,
                           std::function<bool()> updateDigitalFunction = []() { return false; },
                           std::function<float()> updateAnalogFunction = []() { return 0.0f; });

    void UpdateAllActions();
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

    InputAction* RegisterAction(const std::string& name, KeyCode positiveKey, KeyCode negativeKey = KeyCode::Unknown);

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

    virtual void SetCursorShow(bool show) const = 0;

    InputAction* RegisterAction(const std::string& name, MouseButtonCode positiveButton, MouseButtonCode negativeButton = MouseButtonCode::Count);
    InputAction* RegisterAction(const std::string& name, MouseMovementCode movement);

protected:
    DigitalState m_Buttons[static_cast<int>(MouseButtonCode::Count)];
    AnalogState m_Movements[static_cast<int>(MouseMovementCode::Count)];
};

}
