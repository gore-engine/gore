#pragma once

#include "Export.h"

#include "Object/Object.h"
#include "Object/Component.h"

#include <vector>

namespace gore
{

class Scene;
class Component;

ENGINE_CLASS(GameObject) final : public Object
{
public:
    GameObject(const GameObject&)            = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&)                 = delete;
    GameObject& operator=(GameObject&&)      = delete;

    void Update();

    [[nodiscard]] Scene* GetScene() const
    {
        return m_Scene;
    }

    template <typename T>
    T* AddComponent();

    template <typename T>
    T* GetComponent();

    template <typename T>
    void RemoveComponent();

private:
    friend class Scene;
    GameObject(std::string name, Scene* scene);
    ~GameObject() override;

    Scene* m_Scene;

    std::vector<Component*> m_Components;
};

template <typename T>
T* GameObject::AddComponent()
{
    T* component = new T(this);
    m_Components.push_back(component);
    return component;
}

// todo: RTTI and dynamic_cast
template <typename T>
T* GameObject::GetComponent()
{
    for (auto& component : m_Components)
    {
        if (dynamic_cast<T*>(component))
            return static_cast<T*>(component);
    }

    return nullptr;
}

template <typename T>
void GameObject::RemoveComponent()
{
    for (auto it = m_Components.begin(); it != m_Components.end(); ++it)
    {
        if (!dynamic_cast<T*>(*it))
            continue;

        delete *it;
        m_Components.erase(it);
        return;
    }
}

} // namespace gore
