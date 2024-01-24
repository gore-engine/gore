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

    std::unordered_set<GameObject*> gameObjectsToDestroy;

    while (!stack.empty())
    {
        auto* current = stack.back();
        stack.pop_back();

        // this prevents the same game object's children from being iterated multiple times
        if (gameObjectsToDestroy.contains(current))
        {
            continue;
        }

        gameObjectsToDestroy.insert(current);
        for (auto const& child : *(current->GetTransform()))
        {
            // m_GameObjectsToDestroy is a comes-for-free visited set, by which the maximum iteration count will be the number of game objects
            if (!gameObjectsToDestroy.contains(child->GetGameObject()))
            {
                stack.push_back(child->GetGameObject());
            }
        }
    }

    auto destroyedGameObjectCount = std::erase_if(m_GameObjects, [&](auto& go)
                                                  { return gameObjectsToDestroy.contains(go); });

    // Now it's time to delete the game object hard
    std::for_each(gameObjectsToDestroy.begin(), gameObjectsToDestroy.end(), [](auto& go)
                  { LOG_STREAM(DEBUG) << "Destroying " << go->GetName() << std::endl;
                    delete go; });

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