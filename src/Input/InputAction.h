#pragma once

#include "Export.h"

#include "InputEnums.h"
#include "InputDevice.h"

#include <string>
#include <functional>

namespace gore
{

ENGINE_CLASS(InputAction)
{
public:
    InputAction(InputAction&& other) noexcept;
    ~InputAction();

    [[nodiscard]] InputType Type() const { return m_Type; }
    [[nodiscard]] const std::string& Name() const { return m_Name; }

    [[nodiscard]] bool Pressed() const;
    [[nodiscard]] bool Released() const;
    [[nodiscard]] bool Held() const;

    [[nodiscard]] float Value() const;
    [[nodiscard]] float Delta() const;

    void UpdateState();

private:
    friend class InputSystem;
    friend class InputDevice;

    InputAction();
    InputAction(InputDevice* device, std::string name, InputType type,
                std::function<bool()> updateDigitalFunction = []() { return false; },
                std::function<float()> updateAnalogFunction = []() { return 0.0f; });

    InputAction& operator=(InputAction&& other) noexcept;

    std::string m_Name;
    InputType m_Type;

    InputDevice* m_Device;

    InputDevice::DigitalState m_DigitalState;
    InputDevice::AnalogState m_AnalogState;

    std::function<bool()> m_UpdateDigitalFunction;
    std::function<float()> m_UpdateAnalogFunction;
};

} // namespace gore
