#include "TestComponent.h"
#include "Object/GameObject.h"

#include <iostream>

TestComponent::TestComponent(gore::GameObject* gameObject) :
    Component(gameObject)
{
    std::cout << "Created TestComponent in GameObject " << gameObject->GetName() << std::endl;
}

TestComponent::~TestComponent()
{
    std::cout << "Destroyed TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}

void TestComponent::Start()
{
    std::cout << "Start TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}

void TestComponent::Update()
{
    std::cout << "Update TestComponent in GameObject " << GetGameObject()->GetName() << std::endl;
}
