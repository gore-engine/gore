#pragma once

#include "Export.h"

#include "InputDevice.h"
#include "Core/System.h"

namespace gore
{

ENGINE_CLASS(InputSystem) : public System
{
public:
    explicit InputSystem(App* app);
    virtual ~InputSystem();

    NON_COPYABLE(InputSystem);

    [[nodiscard]] virtual Keyboard* GetKeyboard() const = 0;
    [[nodiscard]] virtual Mouse* GetMouse() const = 0;

    static const InputSystem* Get();
};

}
