#include "TestComponent.h"
#include "Object/GameObject.h"
#include "Core/Log.h"

TestComponent::TestComponent(gore::GameObject* gameObject) :
    Component(gameObject)
{
    LOG_STREAM(DEBUG) << "Created TestComponent in GameObject " << gameObject->GetName() << std::endl;
}

TestComponent::~TestComponent()
{
    LOG_STREAM(DEBUG) << "Destroyed TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}

void TestComponent::Start()
{
    LOG_STREAM(DEBUG) << "Start TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}

void TestComponent::Update()
{
    // LOG_STREAM(DEBUG) << "Update TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}
