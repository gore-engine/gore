#include "Light.h"

namespace gore
{
Light::Light(GameObject* gameObject) noexcept :
    Component(gameObject),
    m_Type(LightType::Directional),
    m_Data()
{
}

Light::~Light()
{
}

void Light::Start()
{
}

void Light::Update()
{
}
} // namespace gore