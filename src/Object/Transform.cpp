#include "Transform.h"

#include "GameObject.h"
#include "Math/Quaternion.h"

#include "rtm/matrix3x4f.h"
#include "rtm/qvf.h"
#include "rtm/qvvf.h"

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
    // LOG_STREAM(DEBUG) << "Created Transform in GameObject " << GetGameObject()->GetName() << std::endl;
}

void Transform::Update()
{
    // this->RotateAroundAxisInWorldSpace(Vector3::Up, 0.01f);
    // LOG_STREAM(DEBUG) << "Update Transform in GameObject: " << GetGameObject()->GetName()
    //                   << "  Position: " << m_LocalPosition
    //                   << "  Rotation: " << m_LocalRotation
    //                   << "  Scale: " << m_LocalScale << std::endl;
}

Vector3 Transform::GetLocalPosition() const
{
    return m_LocalPosition;
}

Vector3 Transform::GetLocalScale() const
{
    return m_LocalScale;
}

Quaternion Transform::GetLocalRotation() const
{
    return m_LocalRotation;
}

// Quaternion.ToEuler() has not been implemented
// Vector3 Transform::GetLocalEulerAngles() const
//{
//    return m_LocalRotation.ToEuler();
//}

void Transform::RotateAroundAxisInWorldSpace(const Vector3& axisInWorldSpace, float angle)
{
    auto q = Quaternion::CreateFromAxisAngle(axisInWorldSpace, angle);
    m_LocalRotation = q * m_LocalRotation;
    m_LocalPosition = static_cast<Vector3>(rtm::quat_mul_vector3(static_cast<Vector3::SIMDValueType>(m_LocalPosition), q));
}

Matrix4x4 Transform::GetLocalToWorldMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(m_LocalRotation, m_LocalPosition, m_LocalScale));
}

Matrix4x4 Transform::GetLocalToWorldMatrixIgnoreScale() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(m_LocalRotation, m_LocalPosition));
}

Matrix4x4 Transform::GetWorldToLocalMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(rtm::qvv_inverse(rtm::qvvf{.rotation = m_LocalRotation, .translation = m_LocalPosition, .scale = m_LocalScale})));
}

Matrix4x4 Transform::GetWorldToLocalMatrixIgnoreScale() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(rtm::qv_inverse(rtm::qvf{.rotation = m_LocalRotation, .translation = m_LocalPosition})));
}

} // namespace gore
