#include "PeriodicallyChangeWorldTRS.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"
#include "Math/Vector3.h"
#include "Math/Constants.h"

PeriodicallyChangeWorldTRS::PeriodicallyChangeWorldTRS(gore::GameObject* gameObject) :
    gore::Component(gameObject),
    m_Period(1.0f),
    m_TimePassed(0.0f),
    m_CurrentIndex(0)
{
}

PeriodicallyChangeWorldTRS::~PeriodicallyChangeWorldTRS() = default;

void PeriodicallyChangeWorldTRS::Start()
{
}

void PeriodicallyChangeWorldTRS::Update()
{
    m_TimePassed += GetDeltaTime();
    if (m_TimePassed >= m_Period)
    {
        m_CurrentIndex++;
        LOG_STREAM(INFO) << "PeriodicallyChangeWorldTRS Period triggered." << std::endl;
        m_TimePassed = 0.0f;

        auto transform = GetGameObject()->GetTransform();
        //        transform->SetWorldPosition(transform->GetWorldPosition() + gore::Vector3::Up);
        transform->SetWorldRotation(gore::Quaternion::FromAxisAngle(gore::Vector3::Right, gore::math::constants::PI / 10 * m_CurrentIndex));
    }
}