#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Object.h"
#include "Object/Component.h"
#include "Object/Transform.h"

#include <vector>

namespace gore
{

class Scene;
class Component;

ENGINE_CLASS(GameObject) final : public Object
{
public:
    NON_COPYABLE(GameObject);

    void Update();

    [[nodiscard]] Scene* GetScene() const
    {
        return m_Scene;
    }

public:
    template <typename T>
    Component::SelfOrDerivedTypePointer<T> AddComponent();
    template <typename T>
    // should this be void (i.e. SelfOrDerivedTypeNoReturnValue) instead of T*?
    Component::SelfOrDerivedTypePointer<T> AddComponent(T * component);
#if !COMPILER_GCC
    template <>
    Component::SelfOrDerivedTypePointer<Transform> AddComponent<Transform>();
    template <>
    Component::SelfOrDerivedTypePointer<Transform> AddComponent<Transform>(Transform* inpTransform);
#endif // !COMPILER_GCC


    template <typename T>
    Component::SelfOrDerivedTypePointer<T> GetComponent();
    // Should we have this?
    // template <typename T>
    // Component::SelfOrDerivedTypeReference<T> GetComponent();

    template <typename T>
    Component::SelfOrDerivedTypeNoReturnValue<T> RemoveComponent();
#if !COMPILER_GCC
    template <>
    Component::SelfOrDerivedTypeNoReturnValue<Transform> RemoveComponent<Transform>() noexcept(false);
#endif // !COMPILER_GCC

private:
    friend class Scene;
    GameObject(std::string name, Scene * scene);
    ~GameObject() override;

    Scene* m_Scene;

    std::vector<Component*> m_Components;

    public:
    const Transform* transform;
};

#if COMPILER_GCC
template <>
inline Component::SelfOrDerivedTypePointer<Transform> GameObject::AddComponent<Transform>();
template <>
Component::SelfOrDerivedTypePointer<Transform> GameObject::AddComponent(Transform* inpTransform);

template <>
Component::SelfOrDerivedTypeNoReturnValue<Transform> GameObject::RemoveComponent<Transform>() noexcept(false);
#endif  // COMPILER_GCC

template <typename T>
Component::SelfOrDerivedTypePointer<T> GameObject::AddComponent()
{
    static_assert(std::is_abstract_v<T> == false, "Cannot instantiate abstract class");
    T* component = new T(this);
    m_Components.push_back(component);
    return component;
}

template <typename T>
Component::SelfOrDerivedTypePointer<T> GameObject::AddComponent(T* component)
{
    m_Components.push_back(component);
    return component;
}

// todo: RTTI and dynamic_cast
template <typename T>
Component::SelfOrDerivedTypePointer<T> GameObject::GetComponent()
{
    for (auto& component : m_Components)
    {
        if (dynamic_cast<T*>(component))
            return static_cast<T*>(component);
    }

    return nullptr;
}

template <typename T>
Component::SelfOrDerivedTypeNoReturnValue<T> GameObject::RemoveComponent()
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
