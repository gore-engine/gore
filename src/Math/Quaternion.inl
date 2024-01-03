#pragma once

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Quaternion operator+(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return rtm::vector_add(Q1.m_Q, Q2.m_Q);
}

inline Quaternion operator-(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return rtm::vector_sub(Q1.m_Q, Q2.m_Q);
}

inline Quaternion operator*(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return rtm::quat_mul(Q1.m_Q, Q2.m_Q);
}

inline Quaternion operator*(const Quaternion& Q, float S) noexcept
{
    return rtm::vector_mul(
        rtm::quat_load(reinterpret_cast<const float*>(&Q)),
        S);
}

inline Quaternion operator/(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    Quaternion::SIMDValueType q2conj = rtm::vector_div(
        rtm::quat_conjugate(Q2.m_Q),
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(Q2.m_Q))));
    return rtm::quat_mul(Q1.m_Q, q2conj);
}

inline Quaternion operator*(float S, const Quaternion& Q) noexcept
{
    return rtm::vector_mul(
        rtm::quat_load(reinterpret_cast<const float*>(&Q)),
        S);
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
    return rtm::quat_neg(m_Q);
}

//------------------------------------------------------------------------------
// Quaternion operations
//------------------------------------------------------------------------------

inline float Quaternion::Length() const noexcept
{
    return static_cast<float>(rtm::quat_length(m_Q));
}

inline float Quaternion::LengthSquared() const noexcept
{
    return static_cast<float>(rtm::quat_length_squared(m_Q));
}

inline void Quaternion::Normalize() noexcept
{
    m_Q = rtm::quat_normalize(m_Q);
}

inline void Quaternion::Normalize(Quaternion& result) const noexcept
{
    result.m_Q = rtm::quat_normalize(m_Q);
}

inline void Quaternion::Conjugate() noexcept
{
    m_Q = rtm::quat_conjugate(m_Q);
}

inline void Quaternion::Conjugate(Quaternion& result) const noexcept
{
    result.m_Q = rtm::quat_conjugate(m_Q);
}

inline void Quaternion::Inverse(Quaternion& result) const noexcept
{
    result.m_Q = rtm::quat_conjugate(m_Q);
    result.m_Q = rtm::vector_div(
        result.m_Q,
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(m_Q))));
}

inline float Quaternion::Dot(const Quaternion& q) const noexcept
{
    return static_cast<float>(rtm::quat_dot(m_Q, q.m_Q));
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
    return rtm::quat_from_axis_angle(static_cast<Vector3::SIMDValueType>(axis), angle);
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    return rtm::quat_from_euler(pitch, yaw, roll);
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(const Vector3& angles) noexcept
{
    return rtm::quat_from_euler(angles.x, angles.y, angles.z);
}

// inline Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& M) noexcept
// {
//     throw std::runtime_error("Not implemented");
// }

inline void Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept
{
    result.m_Q = rtm::quat_lerp(q1.m_Q, q2.m_Q, t);
}

inline Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return rtm::quat_lerp(q1.m_Q, q2.m_Q, t);
}

inline void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept
{
    result.m_Q = rtm::quat_slerp(q1.m_Q, q2.m_Q, t);
}

inline Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return rtm::quat_slerp(q1.m_Q, q2.m_Q, t);
}

inline void Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept
{
    result.m_Q = rtm::quat_mul(q1.m_Q, q2.m_Q);
}

inline Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return rtm::quat_mul(q1.m_Q, q2.m_Q);
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

// inline float Quaternion::Angle(const Quaternion& q1, const Quaternion& q2) noexcept
// {
//     throw std::runtime_error("Not implemented");
// }
