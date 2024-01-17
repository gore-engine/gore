#pragma once

#include "Object/Component.h"

class PeriodicallyChangeWorldTRS final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(PeriodicallyChangeWorldTRS);

    float m_Period;

private:
    int m_CurrentIndex;
    float m_TimePassed;
};