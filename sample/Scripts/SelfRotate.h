#pragma once

#include "Object/Component.h"
#include "Math/Vector3.h"
#include "Input/InputDevice.h"

class SelfRotate final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(SelfRotate);

    gore::Vector3 m_RotateAxis;

private:
    gore::Keyboard* m_Keyboard;
    bool m_IsEnabled;
};