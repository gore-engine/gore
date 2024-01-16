#include "SelfRotate.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"

SelfRotate::SelfRotate(gore::GameObject* gameObject) :
    Component(gameObject),
    m_RotateAxis(gore::Vector3::Up)
{
}

SelfRotate::~SelfRotate() = default;

void SelfRotate::Start()
{
}

void SelfRotate::Update()
{
    float deltaTime = GetDeltaTime();

    GetGameObject()->GetTransform()->RotateAroundAxis(m_RotateAxis, deltaTime);
}