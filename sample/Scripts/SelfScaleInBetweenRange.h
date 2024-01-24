#pragma once

#include "Object/Component.h"

class SelfScaleInBetweenRange final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(SelfScaleInBetweenRange);

    void SetMinMaxScale(float minScale, float maxScale);

    float m_MinScale;
    float m_MaxScale;
    float m_Speed;
    float m_CurrentScale;
};