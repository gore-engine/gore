#pragma once

#include "Object/Component.h"
#include "Math/Vector3.h"

class SelfRotate final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(SelfRotate);

    gore::Vector3 m_RotateAxis;
};