#include "Prefix.h"

#include "GameObject.h"
#include "Scene/Scene.h"
#include "Object/Component.h"

#include <utility>
#include <iostream>

namespace gore
{

GameObject::GameObject(std::string name, Scene* scene) :
    Object(std::move(name)),
    m_Scene(scene),
    m_Components()
{
}

GameObject::~GameObject()
{
    for (auto& component : m_Components)
        delete component;
}

void GameObject::Update()
{
    for (auto& component : m_Components)
    {
        if (!component->m_Started)
        {
            component->Start();
            component->m_Started = true;
        }
        component->Update();
    }
}

template <>
void GameObject::RemoveComponent<Transform>() noexcept(false)
{
    LOG_STREAM(ERROR) << "Cannot remove Transform component from GameObject" << std::endl;
    throw std::runtime_error("Cannot remove Transform component from GameObject");
}

} // namespace gore