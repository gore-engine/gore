#pragma once

#include "Object/Component.h"

class SelfDestroyAfterSeconds final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(SelfDestroyAfterSeconds);

public:
    float m_SecondsToLive;
};