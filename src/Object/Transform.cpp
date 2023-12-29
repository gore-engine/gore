#include "Transform.h"

#include "GameObject.h"

namespace gore
{

void Transform::SetLocalPosition(const Vector3& position)
{
    this->m_LocalPosition = position;
}

void Transform::SetLocalScale(const Vector3& scale)
{
    this->m_LocalScale = scale;
}

void Transform::SetLocalRotation(const Quaternion& rotation)
{
    this->m_LocalRotation = rotation;
}

void Transform::SetLocalEulerAngles(const Vector3& eulerAngles)
{
    m_LocalRotation = Quaternion::CreateFromYawPitchRoll(eulerAngles.y, eulerAngles.x, eulerAngles.z);
}

void Transform::Start()
{
    LOG_STREAM(DEBUG) << "Created Transform in GameObject " << GetGameObject()->GetName() << std::endl;
}

void Transform::Update()
{
    LOG_STREAM(DEBUG) << "Update Transform in GameObject " << GetGameObject()->GetName() << m_LocalPosition.x << std::endl;
}

} // namespace gore
