#pragma once

#include "Export.h"

namespace gore
{

class GameObject;

ENGINE_CLASS(Component)
{
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
    explicit Component(GameObject* gameObject);
    virtual ~Component();

private:
    GameObject* m_GameObject;

    bool m_Started;
};

} // namespace gore
