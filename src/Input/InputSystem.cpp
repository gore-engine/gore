#include "Prefix.h"

#include "InputSystem.h"

#include "Core/App.h"

namespace gore
{

static InputSystem* s_InputSystem = nullptr;

InputSystem::InputSystem(App* app) :
    System(app)
{
    s_InputSystem = this;
}

InputSystem::~InputSystem()
{
    s_InputSystem = nullptr;
}

const InputSystem* InputSystem::Get()
{
    return s_InputSystem;
}

} // namespace gore