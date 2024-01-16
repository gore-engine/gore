#pragma once

#include "Export.h"
#include "Utilities/Concepts.h"

#include <type_traits>
#include <concepts>

namespace gore
{

class GameObject;

ENGINE_CLASS(Component)
{
public:
    template <typename TC>
    using SelfOrDerivedTypePointer = std::enable_if_t<IsComponentOrDerivedType<TC>, TC*>;

    template <typename TC>
    using SelfOrDerivedTypeReference = std::enable_if_t<IsComponentOrDerivedType<TC>, TC&>;

    template <typename TC>
    using SelfOrDerivedTypeNoReturnValue = std::enable_if_t<IsComponentOrDerivedType<TC>, void>;

public:
    NON_COPYABLE(Component);

    virtual void Start()  = 0;
    virtual void Update() = 0;

    [[nodiscard]] GameObject* GetGameObject() const
    {
        return m_GameObject;
    }

protected:
    friend class GameObject;
    explicit Component(GameObject * gameObject);
    virtual ~Component();

protected:
    GameObject* m_GameObject;

    bool m_Started;
};

} // namespace gore
