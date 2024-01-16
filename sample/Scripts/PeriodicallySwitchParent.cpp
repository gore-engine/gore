#include "PeriodicallySwitchParent.h"

#include "Core/Time.h"
#include "Object/GameObject.h"

PeriodicallySwitchParent::PeriodicallySwitchParent(gore::GameObject* gameObject) :
    Component(gameObject),
    m_ParentA(nullptr),
    m_ParentB(nullptr),
    m_Time(0.0f),
    m_SwitchInterval(1.0f),
    m_Switched(false),
    m_RecalculateLocalPosition(true)
{
}

PeriodicallySwitchParent::~PeriodicallySwitchParent() = default;

void PeriodicallySwitchParent::Start()
{
}

void PeriodicallySwitchParent::Update()
{
    m_Time += GetDeltaTime();

    if (m_Time >= m_SwitchInterval)
    {
        m_Time     = 0.0f;
        m_Switched = !m_Switched;

        if (m_Switched)
        {
            GetGameObject()->GetTransform()->SetParent(m_ParentA, m_RecalculateLocalPosition);
            LOG_STREAM(DEBUG) << "Switched parent to "
                              << (m_ParentA == nullptr ? "Global space" : m_ParentA->GetGameObject()->GetName())
                              << std::endl;
        }
        else
        {
            GetGameObject()->GetTransform()->SetParent(m_ParentB, m_RecalculateLocalPosition);
            LOG_STREAM(DEBUG) << "Switched parent to "
                              << (m_ParentB == nullptr ? "Global space" : m_ParentB->GetGameObject()->GetName())
                              << std::endl;
        }
    }
}
void PeriodicallySwitchParent::SetParentAB(gore::Transform* parentA, gore::Transform* parentB)
{
    m_ParentA = parentA;
    m_ParentB = parentB;
}
