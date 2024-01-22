#pragma once

#include "Object/Component.h"

#include <vector>

class DeleteMultipleGameObjectsAfterSeconds final : public gore::Component
{
public:
    DECLARE_FUNCTIONS_DERIVED_FROM_GORE_COMPONENT(DeleteMultipleGameObjectsAfterSeconds);

    void SetGameObjectsToDelete(const std::vector<gore::GameObject*>& gameObjectToDelete, int count, float delaySeconds);

private:
    std::vector<gore::GameObject*> m_GameObjectsToDelete;
    int m_GameObjectsToDeleteCount;
    float m_DelaySeconds;
};