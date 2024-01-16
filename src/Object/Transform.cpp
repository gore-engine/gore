#include "Transform.h"

#include "GameObject.h"
#include "Math/Quaternion.h"

#include "rtm/matrix3x4f.h"
#include "rtm/qvf.h"
#include "rtm/qvvf.h"

namespace gore
{

void Transform::SetLocalEulerAngles(const Vector3& eulerAngles)
{
    m_LocalTQS.q = Quaternion::CreateFromYawPitchRoll(eulerAngles.y, eulerAngles.x, eulerAngles.z);
}

void Transform::Start()
{
    // LOG_STREAM(DEBUG) << "Created Transform in GameObject " << GetGameObject()->GetName() << std::endl;
}

void Transform::Update()
{
    // this->RotateAroundAxis(Vector3::Up, 0.01f);
    // LOG_STREAM(DEBUG) << "Update Transform in GameObject: " << GetGameObject()->GetName()
    //                   << "  Position: " << m_LocalPosition
    //                   << "  Rotation: " << m_LocalRotation
    //                   << "  Scale: " << m_LocalScale << std::endl;
}


// Quaternion.ToEuler() has not been implemented
// Vector3 Transform::GetLocalEulerAngles() const
//{
//    return m_LocalRotation.ToEuler();
//}

void Transform::RotateAroundAxis(const Vector3& axis, float angle)
{
    m_LocalTQS.q = Quaternion::CreateFromAxisAngle(axis, angle) * m_LocalTQS.q;
}

Matrix4x4 Transform::GetLocalToWorldMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(m_LocalTQS.q, m_LocalTQS.t, m_LocalTQS.s));
}

Matrix4x4 Transform::GetLocalToWorldMatrixIgnoreScale() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(m_LocalTQS.q, m_LocalTQS.t));
}

Matrix4x4 Transform::GetWorldToLocalMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(rtm::qvv_inverse(rtm::qvvf{.rotation = m_LocalTQS.q, .translation = m_LocalTQS.t, .scale = m_LocalTQS.s})));
}

Matrix4x4 Transform::GetWorldToLocalMatrixIgnoreScale() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(rtm::qv_inverse(rtm::qvf{.rotation = m_LocalTQS.q, .translation = m_LocalTQS.t})));
}

} // namespace gore
