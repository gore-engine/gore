#include "DeleteMultipleGameObjectsAfterSeconds.h"

#include "Core/Time.h"
#include "Scene/Scene.h"
#include "Object/GameObject.h"

DeleteMultipleGameObjectsAfterSeconds::DeleteMultipleGameObjectsAfterSeconds(gore::GameObject* gameObject) :
    Component(gameObject),
    m_GameObjectsToDelete(),
    m_GameObjectsToDeleteCount(0),
    m_DelaySeconds(2.0f)
{
}

DeleteMultipleGameObjectsAfterSeconds::~DeleteMultipleGameObjectsAfterSeconds() = default;

void DeleteMultipleGameObjectsAfterSeconds::Start()
{
}

void DeleteMultipleGameObjectsAfterSeconds::Update()
{
    auto deltaTime = GetDeltaTime();
    if (m_GameObjectsToDelete.empty())
        return;

    m_DelaySeconds -= deltaTime;
    if (m_DelaySeconds <= 0.0f)
    {
        GetGameObject()->GetScene()->DestroyMultipleObjects(m_GameObjectsToDelete.data(), m_GameObjectsToDeleteCount);
        m_GameObjectsToDelete.clear();
        GetGameObject()->RemoveComponent<DeleteMultipleGameObjectsAfterSeconds>();
    }
}
void DeleteMultipleGameObjectsAfterSeconds::SetGameObjectsToDelete(const std::vector<gore::GameObject*>& gameObjectToDelete, int count, float delaySeconds)
{
    m_GameObjectsToDelete      = gameObjectToDelete;
    m_GameObjectsToDeleteCount = count;
    m_DelaySeconds             = delaySeconds;
}
