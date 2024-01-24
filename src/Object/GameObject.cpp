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
    m_Transform(),
    m_Components()
{
    AddComponent<Transform>();
    m_Transform = reinterpret_cast<Transform*>(m_Components[0]);
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
void GameObject::Destroy()
{
    m_Scene->DestroyObject(this);
}

template <>
Component::SelfOrDerivedTypePointer<Transform> GameObject::AddComponent<Transform>()
{
    if (m_Transform != nullptr)
    {
        LOG_STREAM(ERROR) << "Cannot add more than one Transform component to GameObject. "
                          << "This operation will do nothing." << std::endl;
        return m_Transform;
    }

    auto pTransform = new Transform(this);
    m_Components.push_back(pTransform);
    return pTransform;
}
template <>
Component::SelfOrDerivedTypePointer<Transform> GameObject::AddComponent(Transform* inpTransform)
{
    if (m_Transform != nullptr)
    {
        LOG_STREAM(ERROR) << "Cannot add more than one Transform component to GameObject. "
                          << "This operation will do nothing." << std::endl;
        return m_Transform;
    }

    m_Components.push_back(inpTransform);
    return inpTransform;
}

template <>
Component::SelfOrDerivedTypeNoReturnValue<Transform> GameObject::RemoveComponent<Transform>() noexcept(false)
{
    LOG_STREAM(ERROR) << "Cannot remove Transform component from GameObject" << std::endl;
    throw std::runtime_error("Cannot remove Transform component from GameObject");
}

} // namespace gore