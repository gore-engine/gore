#include "Quaternion.h"

#include "Math/Constants.h"

#include "Vector3.h"
#include "Matrix4x4.h"

#include "rtm/impl/vector_common.h"
#include "rtm/vector4f.h"
#include "rtm/quatf.h"
#include "rtm/matrix3x4f.h"

namespace gore
{

using namespace rtm;

std::ostream& operator<<(std::ostream& os, const Quaternion& q) noexcept
{
    return os << "Quaternion(" << static_cast<float>(quat_get_x(q))
              << ", " << static_cast<float>(quat_get_y(q))
              << ", " << static_cast<float>(quat_get_z(q))
              << ", " << static_cast<float>(quat_get_w(q)) << ")";
}

Quaternion::operator SIMDValueType() const noexcept
{
    return m_Q;
}

Quaternion::Quaternion(SIMDValueType F) noexcept :
    m_Q(F)
{
}

//------------------------------------------------------------------------------
// Unary operators
//------------------------------------------------------------------------------

inline Quaternion Quaternion::operator+() const noexcept
{
    return *this;
}

inline Quaternion Quaternion::operator-() const noexcept
{
    return quat_neg(m_Q);
}

//------------------------------------------------------------------------------
// Quaternion operations
//------------------------------------------------------------------------------

inline float Quaternion::Length() const noexcept
{
    return static_cast<float>(quat_length(m_Q));
}

inline float Quaternion::LengthSquared() const noexcept
{
    return static_cast<float>(quat_length_squared(m_Q));
}

inline void Quaternion::Normalize() noexcept
{
    m_Q = quat_normalize(m_Q);
}

inline void Quaternion::Normalize(Quaternion& result) const noexcept
{
    result.m_Q = quat_normalize(m_Q);
}

inline void Quaternion::Conjugate() noexcept
{
    m_Q = quat_conjugate(m_Q);
}

inline void Quaternion::Conjugate(Quaternion& result) const noexcept
{
    result.m_Q = quat_conjugate(m_Q);
}

inline void Quaternion::Inverse(Quaternion& result) const noexcept
{
    result.m_Q = quat_conjugate(m_Q);
    result.m_Q = vector_div(
        result.m_Q,
        vector_set(static_cast<float>(quat_length_squared(m_Q))));
}

inline float Quaternion::Dot(const Quaternion& q) const noexcept
{
    return static_cast<float>(quat_dot(m_Q, q.m_Q));
}

inline void Quaternion::RotateTowards(const Quaternion& target, float maxAngle) noexcept
{
    RotateTowards(target, maxAngle, *this);
}

// inline Vector3 Quaternion::ToEuler() const noexcept
//{
// }

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float angle) noexcept
{
    return quat_from_axis_angle(static_cast<Vector3::SIMDValueType>(axis), angle);
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    return quat_from_euler(pitch, yaw, roll);
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(const Vector3& angles) noexcept
{
    return quat_from_euler(angles.x, angles.y, angles.z);
}

inline Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& M) noexcept
{
    throw std::runtime_error("Not implemented");
}

inline void Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept
{
    result.m_Q = quat_lerp(q1.m_Q, q2.m_Q, t);
}

inline Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return quat_lerp(q1.m_Q, q2.m_Q, t);
}

inline void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept
{
    result.m_Q = quat_slerp(q1.m_Q, q2.m_Q, t);
}

inline Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return quat_slerp(q1.m_Q, q2.m_Q, t);
}

inline void Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept
{
    result.m_Q = quat_mul(q1.m_Q, q2.m_Q);
}

inline Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return quat_mul(q1.m_Q, q2.m_Q);
}

inline Quaternion Quaternion::FromToRotation(const Vector3& fromDir, const Vector3& toDir) noexcept
{
    Quaternion result;
    FromToRotation(fromDir, toDir, result);
    return result;
}

inline Quaternion Quaternion::LookRotation(const Vector3& forward, const Vector3& up) noexcept
{
    Quaternion result;
    LookRotation(forward, up, result);
    return result;
}

inline float Quaternion::Angle(const Quaternion& q1, const Quaternion& q2) noexcept
{
    throw std::runtime_error("Not implemented");
}

void Quaternion::RotateTowards(const Quaternion& target, float maxAngle, Quaternion& result) const noexcept
{
    using SIMDVector = rtm::vector4f;

    const SIMDVector T = m_Q;

    // We can use the conjugate here instead of inverse assuming q1 & q2 are normalized.
    const SIMDVector R = quat_mul(quat_conjugate(T), target);

    const float rs      = quat_get_w(R);
    const float length3 = vector_length3(R);
    const float angle   = 2.f * atan2f(length3, rs);
    if (angle > maxAngle)
    {
        const SIMDVector delta = quat_from_axis_angle(R, maxAngle);
        const SIMDVector Q     = quat_mul(delta, T);
        result.m_Q             = Q;
    }
    else
    {
        // Don't overshoot.
        result = target;
    }
}


void Quaternion::FromToRotation(const Vector3& fromDir, const Vector3& toDir, Quaternion& result) noexcept
{
    // Melax, "The Shortest Arc Quaternion", Game Programming Gems, Charles River Media (2000).

    using SIMDVector = rtm::vector4f;

    const SIMDVector F = vector_normalize3(fromDir);
    const SIMDVector T = vector_normalize3(toDir);

    const float dot = vector_get_x(vector_dot3(F, T));
    if (dot >= 1.f)
    {
        result = Identity;
    }
    else if (dot <= -1.f)
    {
        SIMDVector axis = vector_cross3(F, Vector3::Right);
        if (vector_all_near_equal3(
                vector_set(static_cast<float>(vector_length_squared3(axis))),
                vector_zero()))
        {
            axis = vector_cross3(F, Vector3::Up);
        }

        const SIMDVector Q = quat_from_axis_angle(axis, math::constants::PI);
        result.m_Q         = Q;
    }
    else
    {
        const SIMDVector C = vector_cross3(F, T);
        const float s      = sqrtf((1.f + dot) * 2.f);
        vector_div(C, vector_set(s));
        result.m_Q = C;
    }
}

void Quaternion::LookRotation(const Vector3& forward, const Vector3& up, Quaternion& result) noexcept
{
    using SIMDVector = rtm::vector4f;

    Quaternion q1;
    FromToRotation(Vector3::Forward, forward, q1);

    const SIMDVector C = vector_cross3(forward, up);
    if (vector_all_near_equal3(vector_length_squared3(C), vector_zero()))
    {
        // forward and up are co-linear
        result = q1;
        return;
    }

    SIMDVector U = quat_mul(q1, Vector3::Up);

    Quaternion q2;
    FromToRotation(Vector3(U), up, q2);

    result.m_Q = quat_mul(q2, q1);
}

} // namespace gore