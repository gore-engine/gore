#include "Prefix.h"

#include "InputDevice.h"
#include "InputAction.h"

#include <cstring>
#include <utility>

namespace gore
{

InputDevice::InputDevice() :
    m_Actions()
{
}

InputDevice::~InputDevice()
{
}

const InputAction* InputDevice::GetAction(const std::string& name) const
{
    auto it = m_Actions.find(name);
    if (it == m_Actions.end())
    {
        return nullptr;
    }

    return &it->second;
}

void InputDevice::UpdateAllActions()
{
    for (auto& action : m_Actions)
    {
        action.second.UpdateState();
    }
}

InputAction* InputDevice::AddAction(const std::string& name, InputType type, std::function<bool()> updateDigitalFunction, std::function<float()> updateAnalogFunction)
{
    const auto& res = m_Actions.emplace(name, InputAction(this, name, type, std::move(updateDigitalFunction), std::move(updateAnalogFunction)));

    return &res.first->second;
}

Keyboard::Keyboard() :
    InputDevice(),
    m_Keys()
{
    memset(m_Keys, 0, sizeof(m_Keys));
}

Keyboard::~Keyboard()
{
}

bool Keyboard::KeyState(KeyCode key) const
{
    const DigitalState& state = m_Keys[static_cast<int>(key)];
    return state.state;
}

bool Keyboard::KeyPressed(KeyCode key) const
{
    const DigitalState& state = m_Keys[static_cast<int>(key)];
    return state.state && !state.lastState;
}

bool Keyboard::KeyReleased(KeyCode key) const
{
    const DigitalState& state = m_Keys[static_cast<int>(key)];
    return !state.state && state.lastState;
}

InputAction* Keyboard::RegisterAction(const std::string& name, KeyCode positiveKey, KeyCode negativeKey)
{
    std::function<bool()> digitalUpdate = [this, positiveKey]()
    { return KeyState(positiveKey); };
    std::function<float()> analogUpdate = [this, positiveKey, negativeKey]()
    {
        float result = 0.0f;
        if (KeyState(positiveKey))
            result += 1.0f;
        if (KeyState(negativeKey))
            result -= 1.0f;
        return result;
    };

    return AddAction(name, InputType::Digital, digitalUpdate, analogUpdate);
}

Mouse::Mouse() :
    InputDevice(),
    m_Buttons(),
    m_Movements()
{
    memset(m_Buttons, 0, sizeof(m_Buttons));
    memset(m_Movements, 0, sizeof(m_Movements));
}

Mouse::~Mouse()
{
}

bool Mouse::ButtonState(MouseButtonCode button) const
{
    const DigitalState& state = m_Buttons[static_cast<int>(button)];
    return state.state;
}

bool Mouse::ButtonPressed(MouseButtonCode button) const
{
    const DigitalState& state = m_Buttons[static_cast<int>(button)];
    return state.state && !state.lastState;
}

bool Mouse::ButtonReleased(MouseButtonCode button) const
{
    const DigitalState& state = m_Buttons[static_cast<int>(button)];
    return !state.state && state.lastState;
}

float Mouse::Get(MouseMovementCode movement) const
{
    const AnalogState& state = m_Movements[static_cast<int>(movement)];
    return state.state;
}

float Mouse::GetDelta(MouseMovementCode movement) const
{
    const AnalogState& state = m_Movements[static_cast<int>(movement)];
    return state.state - state.lastState;
}

InputAction* Mouse::RegisterAction(const std::string& name, MouseButtonCode positiveButton, MouseButtonCode negativeButton)
{
    std::function<bool()> digitalUpdate = [this, positiveButton]()
    { return ButtonState(positiveButton); };
    std::function<float()> analogUpdate = [this, positiveButton, negativeButton]()
    {
        float result = 0.0f;
        if (ButtonState(positiveButton))
            result += 1.0f;
        if (static_cast<int>(negativeButton) < static_cast<int>(MouseButtonCode::Count))
        {
            if (ButtonState(negativeButton))
                result -= 1.0f;
        }
        return result;
    };
    return AddAction(name, InputType::Digital, digitalUpdate, analogUpdate);
}

InputAction* Mouse::RegisterAction(const std::string& name, MouseMovementCode movement)
{
    std::function<bool()> digitalUpdate = [this, movement]()
    { return GetDelta(movement) > 0; };
    std::function<float()> analogUpdate = [this, movement]()
    { return Get(movement); };

    return AddAction(name, InputType::Analog, digitalUpdate, analogUpdate);
}

} // namespace gore
