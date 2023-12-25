#pragma once

#include "Object/Component.h"

class TestComponent final : public gore::Component
{
public:
    explicit TestComponent(gore::GameObject* gameObject);
    ~TestComponent() override;

    TestComponent(const TestComponent&)            = delete;
    TestComponent& operator=(const TestComponent&) = delete;
    TestComponent(TestComponent&&)                 = delete;
    TestComponent& operator=(TestComponent&&)      = delete;

    void Start() override;
    void Update() override;
};
