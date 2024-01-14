#include "Prefix.h"

#include "InputAction.h"

#include <utility>

namespace gore
{

InputAction::InputAction() :
    m_Name(),
    m_Type(InputType::Digital),
    m_Device(nullptr),
    m_DigitalState(),
    m_AnalogState(),
    m_UpdateDigitalFunction([]() { return false; }),
    m_UpdateAnalogFunction([]() { return 0.0f; })
{
}

InputAction::InputAction(InputDevice* device, std::string name, InputType type,
                         std::function<bool()> updateDigitalFunction,
                         std::function<float()> updateAnalogFunction) :
    m_Name(std::move(name)),
    m_Type(type),
    m_Device(device),
    m_DigitalState(),
    m_AnalogState(),
    m_UpdateDigitalFunction(std::move(updateDigitalFunction)),
    m_UpdateAnalogFunction(std::move(updateAnalogFunction))
{
}

InputAction::InputAction(InputAction&& other) noexcept :
    m_Name(std::move(other.m_Name)),
    m_Type(other.m_Type),
    m_Device(other.m_Device),
    m_DigitalState(other.m_DigitalState),
    m_AnalogState(other.m_AnalogState),
    m_UpdateDigitalFunction(std::move(other.m_UpdateDigitalFunction)),
    m_UpdateAnalogFunction(std::move(other.m_UpdateAnalogFunction))
{
}

InputAction::~InputAction()
{
}

InputAction& InputAction::operator=(InputAction&& other) noexcept
{
    m_Name = std::move(other.m_Name);
    m_Type = other.m_Type;
    m_Device = other.m_Device;
    m_DigitalState = other.m_DigitalState;
    m_AnalogState = other.m_AnalogState;
    m_UpdateDigitalFunction = std::move(other.m_UpdateDigitalFunction);
    m_UpdateAnalogFunction = std::move(other.m_UpdateAnalogFunction);

    return *this;
}

bool InputAction::Pressed() const
{
    return !m_DigitalState.lastState && m_DigitalState.state;
}

bool InputAction::Released() const
{
    return m_DigitalState.lastState && !m_DigitalState.state;
}

bool InputAction::Held() const
{
    return m_DigitalState.state;
}

float InputAction::Value() const
{
    return m_AnalogState.state;
}

float InputAction::Delta() const
{
    return m_AnalogState.state - m_AnalogState.lastState;
}

void InputAction::UpdateState()
{
    m_DigitalState.lastState = m_DigitalState.state;
    m_DigitalState.state = m_UpdateDigitalFunction();

    m_AnalogState.lastState = m_AnalogState.state;
    m_AnalogState.state = m_UpdateAnalogFunction();
}

} // namespace gore