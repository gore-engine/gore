#pragma once

#include "Object/Component.h"
#include "Math/Vector3.h"

class SelfMoveBackAndForth final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(SelfMoveBackAndForth);

    gore::Vector3 m_Direction;
    float m_Speed;
    float m_FarthestDistance;
};