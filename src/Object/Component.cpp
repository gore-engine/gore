#include "Prefix.h"

#include "Component.h"
#include "Object/GameObject.h"

namespace gore
{

Component::Component(GameObject* gameObject) :
    m_GameObject(gameObject),
    m_Started(false)
{
}

Component::~Component()
{
}

} // namespace gore