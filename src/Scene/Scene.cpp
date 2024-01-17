#include "Prefix.h"

#include "Scene.h"
#include "Object/GameObject.h"
#include "Object/Transform.h"

#include <utility>
#include <iostream>
#include <algorithm>

namespace gore
{

std::vector<Scene*> Scene::s_CurrentScenes;
Scene* Scene::s_ActiveScene = nullptr;

Scene::Scene(std::string name) :
    m_Name(std::move(name)),
    m_GameObjects()
{
    s_CurrentScenes.push_back(this);
    SetAsActive();
}

Scene::~Scene()
{
    auto it = std::find(s_CurrentScenes.begin(), s_CurrentScenes.end(), this);
    if (it != s_CurrentScenes.end())
    {
        s_CurrentScenes.erase(it);
    }

    if (s_ActiveScene == this)
        s_ActiveScene = s_CurrentScenes.empty() ? nullptr : s_CurrentScenes[0];

    for (auto& gameObject : m_GameObjects)
    {
        delete gameObject;
    }
}

void Scene::Update()
{
    for (auto& gameObject : m_GameObjects)
    {
        gameObject->Update();
    }
}

GameObject* Scene::NewObject(std::string name)
{
    auto* gameObject = new GameObject(std::move(name), this);
    m_GameObjects.push_back(gameObject);
    return gameObject;
}

void Scene::DestroyObject(GameObject* gameObject)
{
    auto newLogicalEnd =
        std::remove_if(m_GameObjects.begin(), m_GameObjects.end(), [gameObject](GameObject* pGameObject)
                       { return pGameObject->GetTransform()->GetRoot()->GetGameObject() == gameObject; });
    std::for_each(newLogicalEnd, m_GameObjects.end(), [](GameObject* pGameObject)
                  { delete pGameObject; });
    m_GameObjects.erase(newLogicalEnd, m_GameObjects.end());
}

GameObject* Scene::FindObject(const std::string& name)
{
    for (auto& gameObject : m_GameObjects)
    {
        if (gameObject->GetName() == name)
            return gameObject;
    }

    return nullptr;
}

void Scene::SetAsActive()
{
    s_ActiveScene = this;
}

Scene* Scene::GetActiveScene()
{
    return s_ActiveScene;
}

} // namespace gore