#pragma once

#include "Export.h"

#include <string>
#include <vector>

namespace gore
{

class GameObject;

ENGINE_CLASS(Scene)
{
public:
    explicit Scene(std::string name);
    ~Scene();

    NON_COPYABLE(Scene);

    [[nodiscard]] const std::string& GetName() const
    {
        return m_Name;
    }
    void SetName(std::string name)
    {
        m_Name = std::move(name);
    }

    void Update();

    GameObject* NewObject(std::string name = "New GameObject");
    // If we have time to implement this
    // GameObject* NewObject(std::string name = "New GameObject", Vector3 position = Vector3::Zero);

    void DestroyObject(GameObject* gameObject);

    GameObject* FindObject(const std::string& name);

    void SetAsActive();
    static Scene* GetActiveScene();

    [[nodiscard]] static std::vector<Scene*> GetScenes()
    {
        return s_CurrentScenes;
    }

private:
    std::string m_Name;

    std::vector<GameObject*> m_GameObjects;

    static std::vector<Scene*> s_CurrentScenes;
    static Scene* s_ActiveScene;
};

} // namespace gore
