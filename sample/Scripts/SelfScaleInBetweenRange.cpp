#include "SelfScaleInBetweenRange.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"

#include <cmath>

SelfScaleInBetweenRange::SelfScaleInBetweenRange(gore::GameObject* gameObject) :
    Component(gameObject),
    m_MinScale(0.5f),
    m_MaxScale(1.5f),
    m_Speed(1.0f),
    m_CurrentScale(1.0f)
{
}

SelfScaleInBetweenRange::~SelfScaleInBetweenRange() = default;

void SelfScaleInBetweenRange::Start()
{
}

void SelfScaleInBetweenRange::Update()
{
    float totalTime = GetTotalTime();

    m_CurrentScale = m_MinScale + sinf(totalTime * m_Speed) * (m_MaxScale - m_MinScale);
    GetGameObject()->GetTransform()->SetLocalScale(gore::Vector3(m_CurrentScale));
}

void SelfScaleInBetweenRange::SetMinMaxScale(float minScale, float maxScale)
{
    m_MinScale = minScale;
    m_MaxScale = maxScale;
}
