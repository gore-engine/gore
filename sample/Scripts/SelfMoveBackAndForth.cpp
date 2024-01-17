#include "SelfMoveBackAndForth.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"

#include <cmath>

SelfMoveBackAndForth::SelfMoveBackAndForth(gore::GameObject* gameObject) :
    Component(gameObject),
    m_Direction(gore::Vector3::Forward),
    m_Speed(1.0f),
    m_FarthestDistance(1.0f)
{
}

SelfMoveBackAndForth::~SelfMoveBackAndForth() = default;

void SelfMoveBackAndForth::Start()
{
}

void SelfMoveBackAndForth::Update()
{
    float totalTime = GetTotalTime();

    gore::Vector3 position = GetGameObject()->GetTransform()->GetLocalPosition();
    position += sinf(totalTime * m_Speed) * m_Direction * m_FarthestDistance * GetDeltaTime();
    GetGameObject()->GetTransform()->SetWorldPosition(position);
}