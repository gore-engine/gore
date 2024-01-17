#include "SelfDestroyAfterSeconds.h"

#include "Core/Time.h"
#include "Object/GameObject.h"
#include "Scene/Scene.h"

SelfDestroyAfterSeconds::SelfDestroyAfterSeconds(gore::GameObject* gameObject)
    : Component(gameObject)
    , m_SecondsToLive(3.0f)
{
}

SelfDestroyAfterSeconds::~SelfDestroyAfterSeconds() = default;

void SelfDestroyAfterSeconds::Start()
{
}

void SelfDestroyAfterSeconds::Update()
{
    m_SecondsToLive -= GetDeltaTime();
    if (m_SecondsToLive <= 0.0f)
    {
        GetGameObject()->Destroy();
    }
}
