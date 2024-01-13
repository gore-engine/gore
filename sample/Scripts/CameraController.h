#pragma once

#include "Object/Component.h"
#include "Input/InputSystem.h"

class CameraController final : public gore::Component
{
public:
    explicit CameraController(gore::GameObject* gameObject);
    ~CameraController() override;

    void Start() override;
    void Update() override;

private:
    const gore::Keyboard* m_Keyboard;
    const gore::Mouse* m_Mouse;

    float m_Yaw;
    float m_Pitch;
    float m_Roll;
};
