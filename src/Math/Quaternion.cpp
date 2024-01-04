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
    float temp[4];
    vector_store(q.m_Q, temp);
    return os << "Quaternion(" << temp[0]
              << ", " << temp[1]
              << ", " << temp[2]
              << ", " << temp[3] << ")";
}

Quaternion::operator SIMDValueType() const noexcept
{
    return m_Q;
}

Quaternion::Quaternion(SIMDValueType F) noexcept :
    m_Q(F)
{
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