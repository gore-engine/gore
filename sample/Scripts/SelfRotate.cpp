#include "SelfRotate.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"
#include "Input/InputSystem.h"

SelfRotate::SelfRotate(gore::GameObject* gameObject) :
    Component(gameObject),
    m_RotateAxis(gore::Vector3::Up),
    m_Keyboard(gore::InputSystem::Get()->GetKeyboard())
{
}

SelfRotate::~SelfRotate() = default;

void SelfRotate::Start()
{
}

void SelfRotate::Update()
{
    if (m_Keyboard->KeyPressed(gore::KeyCode::T))
    {
        m_IsEnabled = !m_IsEnabled;
    }
    if (!m_IsEnabled) return;

    float deltaTime = GetDeltaTime();

    GetGameObject()->GetTransform()->RotateAroundAxis(m_RotateAxis, deltaTime);
}