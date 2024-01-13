#include "Prefix.h"

#include "InputDevice.h"

#include <cstring>

namespace gore
{

InputDevice::~InputDevice()
{
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

} // namespace gore
