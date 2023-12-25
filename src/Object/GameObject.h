#pragma once

#include "Export.h"

#include "Object/Object.h"

namespace gore
{

class Scene;

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

private:
    friend class Scene;
    GameObject(std::string name, Scene* scene);
    ~GameObject() override;

    Scene* m_Scene;
};

} // namespace gore
