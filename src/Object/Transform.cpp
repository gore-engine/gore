#include "Transform.h"

#include "GameObject.h"
#include "Math/Quaternion.h"

#include "rtm/matrix3x4f.h"
#include "rtm/qvf.h"
#include "rtm/qvvf.h"

namespace gore
{

Transform::~Transform()
{
    LOG_STREAM(WARNING) << "Destroyed Transform in GameObject " << GetGameObject()->GetName()
                        << ". Note that the destruction of children objects has NOT been implemented."
                        << std::endl;
}

void Transform::SetLocalEulerAngles(const Vector3& eulerAngles)
{
    m_LocalTQS.q = Quaternion::FromYawPitchRoll(eulerAngles.y, eulerAngles.x, eulerAngles.z);
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

Transform* Transform::Find(const std::string& name, bool recursive) const
{
    auto result = std::find_if(m_Children.begin(), m_Children.end(), [name](Transform* child)
                               { return child->GetGameObject()->GetName() == name; });
    if (result != m_Children.end())
    {
        return *result;
    }
    if (!recursive)
    {
        return nullptr;
    }

    for (auto& child : m_Children)
    {
        auto pTransform = child->Find(name, recursive);
        if (pTransform != nullptr)
            return pTransform;
    }
    return nullptr;
}

Vector3 Transform::GetWorldPosition() const
{
    return GetLocalToWorldTQS().t;
}

void Transform::SetWorldPosition(const Vector3& position)
{
    if (m_Parent == nullptr)
    {
        m_LocalTQS.t = position;
        return;
    }

    m_LocalTQS.t = m_Parent->InverseTransformPoint(position);
}

Vector3 Transform::GetWorldScale() const
{
    return GetLocalToWorldTQS().s;
}

void Transform::SetWorldScale(const Vector3& scale)
{
    if (m_Parent == nullptr)
    {
        m_LocalTQS.s = scale;
        return;
    }

    m_LocalTQS.s = m_Parent->InverseTransformVector3(scale);
}

Quaternion Transform::GetWorldRotation() const
{
    return GetLocalToWorldTQS().q;
}

void Transform::SetWorldRotation(const Quaternion& rotation)
{
    if (m_Parent == nullptr)
    {
        m_LocalTQS.q = rotation;
        return;
    }

    auto parentRotation = m_Parent->GetWorldRotation();
    m_LocalTQS.q        = parentRotation.Inverse() * rotation;
}

void Transform::RotateAroundAxis(const Vector3& axis, float angle)
{
    m_LocalTQS.q = Quaternion::FromAxisAngle(axis, angle) * m_LocalTQS.q;
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
        m_LocalTQS = newParent == nullptr ? this->GetLocalToWorldTQS() : TQS::Mul(this->GetLocalToWorldTQS(), newParent->GetWorldToLocalTQS());
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

Vector3 Transform::TransformPoint(const Vector3& point, bool useScale /* = true */) const
{
    return GetLocalToWorldTQS().MulPoint3(point, useScale);
}

Vector3 Transform::TransformVector3(const Vector3& vector, bool useScale /* = true */) const
{
    return GetLocalToWorldTQS().MulVector3(vector, useScale);
}

Vector3 Transform::InverseTransformPoint(const Vector3& point, bool useScale /* = true */) const
{
    return GetLocalToWorldTQS().InvMulPoint3(point, useScale);
}

Vector3 Transform::InverseTransformVector3(const Vector3& vector, bool useScale /* = true */) const
{
    return GetLocalToWorldTQS().InvMulVector3(vector, useScale);
}

} // namespace gore
