#pragma once

#include "Object/Component.h"

class TestComponent final : public gore::Component
{
public:
    explicit TestComponent(gore::GameObject* gameObject);
    ~TestComponent() override;

    void Start() override;
    void Update() override;
};
