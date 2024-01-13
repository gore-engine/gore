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
    gore::Keyboard* m_Keyboard;
    gore::Mouse* m_Mouse;

    gore::InputAction* m_ActionMoveHorizontal;
    gore::InputAction* m_ActionMoveVertical;
    gore::InputAction* m_ActionMoveUpDown;

    gore::InputAction* m_ActionAllowRotation;

    gore::InputAction* m_ActionRotateHorizontal;
    gore::InputAction* m_ActionRotateVertical;
    gore::InputAction* m_ActionZoom;

    float m_Yaw;
    float m_Pitch;
    float m_Roll;
};
