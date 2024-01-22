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
    m_GameObjectsToDestroy[gameObject] = false;
    return gameObject;
}

void Scene::DestroyObject(GameObject* pGameObject)
{
    DestroyMultipleObjects(&pGameObject, 1);
}

void Scene::DestroyMultipleObjects(GameObject** ppGameObjects, int count)
{
    if (ppGameObjects == nullptr)
    {
        LOG_STREAM(ERROR) << "Cannot destroy nullptr" << std::endl;
        return;
    }

    // dfs by a while loop
    std::vector<GameObject*> stack;
    for (int i = 0; i < count; ++i)
    {
        if (ppGameObjects[i] == nullptr)
        {
            LOG_STREAM(ERROR) << "Cannot destroy nullptr" << std::endl;
            continue;
        }
        if (ppGameObjects[i]->GetScene() != this)
        {
            LOG_STREAM(ERROR) << "Cannot destroy object" << ppGameObjects[i]->GetName() << "from another scene." << std::endl;
            continue;
        }
        stack.push_back(ppGameObjects[i]);
        ppGameObjects[i]->GetTransform()->SetParent(nullptr, false);
    }

    while (!stack.empty())
    {
        auto* current = stack.back();
        stack.pop_back();

        for (auto const& child : *(current->GetTransform()))
        {
            stack.push_back(child->GetGameObject());
        }

        m_GameObjectsToDestroy[current] = true;
    }

    auto destroyedGameObjectCount = std::erase_if(m_GameObjects, [this](auto& go)
                                                  { return m_GameObjectsToDestroy[go]; });

    // Now it's time to delete the game object hard
    std::for_each(m_GameObjectsToDestroy.begin(), m_GameObjectsToDestroy.end(), [](auto& pair)
                  {if (pair.second)
                      {
                          LOG_STREAM(DEBUG) << "Destroying " << pair.first->GetName() << std::endl;
                          delete pair.first;
                      } });

    std::erase_if(m_GameObjectsToDestroy, [](auto& pair)
                  { return pair.second; });

    LOG_STREAM(DEBUG) << "Destroyed " << destroyedGameObjectCount << " game objects" << std::endl;
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