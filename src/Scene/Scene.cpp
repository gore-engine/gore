#include "Prefix.h"

#include "Scene.h"
#include "Object/GameObject.h"

#include <utility>
#include <iostream>

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
    auto it = std::find(m_GameObjects.begin(), m_GameObjects.end(), gameObject);
    if (it != m_GameObjects.end())
    {
        m_GameObjects.erase(it);
        delete gameObject;
    }
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