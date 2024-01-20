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
    // std::remove_if removes the elements by shifting latter elements to the left and leave the higher indices be
    // So if we want to delete the objects, we need to delete them BEFORE the predicate function returns true
    auto newLogicalEnd =
        std::remove_if(m_GameObjects.begin(), m_GameObjects.end(), [gameObject](GameObject* pGameObject)
                       {
                           if (pGameObject == gameObject && pGameObject->GetTransform()->GetParent() != nullptr)
                           {
                               // If the GameObject is the one we want to destroy, we need to remove it from its parent
                               // Setting reCalculateLocalTQS to false will prevent unnecessary calculations
                               pGameObject->GetTransform()->SetParent(nullptr, false);
                           }
                           if (pGameObject == gameObject || pGameObject->GetTransform()->IsChildOf(gameObject->GetTransform(), true))
                           {
                               LOG_STREAM(DEBUG) << "Destroying GameObject " << pGameObject->GetName() << std::endl;
                               delete pGameObject;
                               return true;
                           }
                           return false; });

    LOG_STREAM(DEBUG) << "Destroyed " << (std::distance(newLogicalEnd, m_GameObjects.end()))
                      << " GameObjects" << std::endl;
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