#include "Transform.h"

#include "GameObject.h"

#include "rtm/matrix3x4f.h"

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

inline Vector3 Transform::GetLocalPosition() const
{
    return m_LocalPosition;
}

inline Vector3 Transform::GetLocalScale() const
{
    return m_LocalScale;
}

inline Quaternion Transform::GetLocalRotation() const
{
    return m_LocalRotation;
}

// Quaternion.ToEuler() has not been implemented
// Vector3 Transform::GetLocalEulerAngles() const
//{
//    return m_LocalRotation.ToEuler();
//}

Matrix4x4 Transform::GetLocalToWorldMatrix() const
{
    return rtm::matrix_from_qvv(m_LocalPosition, m_LocalRotation, m_LocalScale);
}

} // namespace gore
