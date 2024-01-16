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

Transform* Transform::GetRoot() const
{
    auto pTransform = const_cast<Transform*>(this);
    while (pTransform->m_Parent != nullptr)
        pTransform = pTransform->m_Parent;
    return pTransform;
}

void Transform::RotateAroundAxis(const Vector3& axis, float angle)
{
    m_LocalTQS.q = Quaternion::CreateFromAxisAngle(axis, angle) * m_LocalTQS.q;
}

Matrix4x4 Transform::GetLocalToWorldMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(static_cast<TQS::SIMDValueType>(GetLocalToWorldTQS())));
}

Matrix4x4 Transform::GetLocalToWorldMatrixIgnoreScale() const
{
    TQS tqsWorld = GetLocalToWorldTQS();
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(tqsWorld.q, tqsWorld.t));
}

Matrix4x4 Transform::GetWorldToLocalMatrix() const
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(static_cast<TQS::SIMDValueType>(GetWorldToLocalTQS())));
}

Matrix4x4 Transform::GetWorldToLocalMatrixIgnoreScale() const
{
    TQS tqsWorld = GetLocalToWorldTQS();
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(rtm::qv_inverse(rtm::qv_set(tqsWorld.q, tqsWorld.t))));
}

void Transform::SetParent(Transform* newParent, bool reCalculateLocalTQS /* = true */)
{
    if (m_Parent == newParent)
        return;

    auto oldParent = m_Parent;

    if (newParent != nullptr)
    {
        // check if newParent is a child of this
        auto it = std::find(newParent->begin(), newParent->end(), this);
        if (it != newParent->m_Children.end())
        {
            LOG_STREAM(ERROR) << "Cannot set parent to a child of this Transform. "
                              << "This operation will do nothing." << std::endl;
            return;
        }
    }

    if (oldParent != nullptr)
    {
        auto it = std::find(m_Parent->begin(), m_Parent->end(), this);
        if (it != m_Parent->m_Children.end())
            m_Parent->m_Children.erase(it);
    }
    if (newParent != nullptr)
    {
        newParent->m_Children.push_back(this);
    }

    if (reCalculateLocalTQS)
    {
        m_LocalTQS = newParent == nullptr ? this->m_LocalTQS : TQS::Mul(this->GetLocalToWorldTQS(), newParent->GetWorldToLocalTQS());
    }

    m_Parent = newParent;
}

TQS Transform::GetLocalToWorldTQS() const
{
    if (m_Parent == nullptr)
    {
        return m_LocalTQS;
    }

    const Transform* current = m_Parent;
    TQS result               = m_LocalTQS;

    while (current != nullptr)
    {
        result  = TQS::Mul(result, current->m_LocalTQS);
        current = current->m_Parent;
    }
    return result;
}

TQS Transform::GetWorldToLocalTQS() const
{
    return GetLocalToWorldTQS().Inverse();
}

} // namespace gore
