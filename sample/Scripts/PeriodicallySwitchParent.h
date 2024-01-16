#pragma once

#include "Object/Component.h"
#include "Object/Transform.h"

class PeriodicallySwitchParent final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(PeriodicallySwitchParent);

    void SetParentAB(gore::Transform* parentA, gore::Transform* parentB);

public:
    gore::Transform* m_ParentA;
    gore::Transform* m_ParentB;
    float m_SwitchInterval;
    bool m_RecalculateLocalPosition;

private:
    float m_Time;
    bool m_Switched;
};
