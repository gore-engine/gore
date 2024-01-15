#include "TestComponent.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"
#include "Core/Log.h"
#include "Core/Time.h"
#include "Math/Quaternion.h"

TestComponent::TestComponent(gore::GameObject* gameObject) :
    Component(gameObject)
{
}

TestComponent::~TestComponent()
{
}

void TestComponent::Start()
{
}

void TestComponent::Update()
{
    float deltaTime = GetDeltaTime();

    gore::Transform* transform = m_GameObject->GetComponent<gore::Transform>();

    transform->RotateAroundAxis(gore::Vector3::Up, deltaTime);
}
