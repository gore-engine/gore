#pragma once

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Quaternion operator+(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return static_cast<Quaternion>(rtm::vector_add(Q1.m_Q, Q2.m_Q));
}

inline Quaternion operator-(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return static_cast<Quaternion>(rtm::vector_sub(Q1.m_Q, Q2.m_Q));
}

inline Quaternion operator*(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    return static_cast<Quaternion>(rtm::quat_mul(Q1.m_Q, Q2.m_Q));
}

inline Quaternion operator*(const Quaternion& Q, float S) noexcept
{
    return static_cast<Quaternion>(rtm::vector_mul(
        rtm::quat_load(reinterpret_cast<const float*>(&Q)),
        S));
}

inline Quaternion operator/(const Quaternion& Q1, const Quaternion& Q2) noexcept
{
    Quaternion::SIMDValueType q2conj = rtm::vector_div(
        rtm::quat_conjugate(Q2.m_Q),
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(Q2.m_Q))));
    return static_cast<Quaternion>(rtm::quat_mul(Q1.m_Q, q2conj));
}

inline Quaternion operator*(float S, const Quaternion& Q) noexcept
{
    return static_cast<Quaternion>(rtm::vector_mul(
        rtm::quat_load(reinterpret_cast<const float*>(&Q)),
        S));
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
    return static_cast<Quaternion>(rtm::quat_neg(m_Q));
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

inline bool Quaternion::IsNormalized() const noexcept
{
    return rtm::quat_is_normalized(m_Q);
}

inline Quaternion Quaternion::Normalized() const noexcept
{
    return static_cast<Quaternion>(rtm::quat_normalize(m_Q));
}

inline void Quaternion::Normalize() noexcept
{
    m_Q = rtm::quat_normalize(m_Q);
}

inline void Quaternion::Normalize(Quaternion& q) noexcept
{
    q.m_Q = rtm::quat_normalize(q.m_Q);
}

inline Quaternion Quaternion::Conjucated() const noexcept
{
    return static_cast<Quaternion>(rtm::quat_conjugate(m_Q));
}

inline void Quaternion::Conjugate() noexcept
{
    m_Q = rtm::quat_conjugate(m_Q);
}

inline void Quaternion::Conjugate(Quaternion& q) noexcept
{
    q.m_Q = rtm::quat_conjugate(q.m_Q);
}

inline Quaternion Quaternion::Inversed() const noexcept
{
    Quaternion result;
    result.m_Q = rtm::quat_conjugate(result.m_Q);
    result.m_Q = rtm::vector_div(
        result.m_Q,
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(result.m_Q))));
    return result;
}

inline void Quaternion::Inverse() noexcept
{
    m_Q = rtm::quat_conjugate(m_Q);
    m_Q = rtm::vector_div(
        m_Q,
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(m_Q))));
}

inline void Quaternion::Inverse(Quaternion& q) noexcept
{
//    if (q.IsNormalized())
//    {
//        q.Conjugate();
//        return;
//    }
    q.m_Q = rtm::quat_conjugate(q.m_Q);
    q.m_Q = rtm::vector_div(
        q.m_Q,
        rtm::vector_set(static_cast<float>(rtm::quat_length_squared(q.m_Q))));
}

inline float Quaternion::Dot(const Quaternion& q) const noexcept
{
    return static_cast<float>(rtm::quat_dot(m_Q, q.m_Q));
}

// inline Vector3 Quaternion::ToEuler() const noexcept
//{
// }

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float angle) noexcept
{
    return static_cast<Quaternion>(rtm::quat_from_axis_angle(static_cast<Vector3::SIMDValueType>(axis), angle));
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    return static_cast<Quaternion>(rtm::quat_from_euler(pitch, yaw, roll));
}

inline Quaternion Quaternion::CreateFromYawPitchRoll(const Vector3& angles) noexcept
{
    return static_cast<Quaternion>(rtm::quat_from_euler(angles.x, angles.y, angles.z));
}

// inline Quaternion Quaternion::CreateFromRotationMatrix(const Matrix4x4& M) noexcept
// {
//     throw std::runtime_error("Not implemented");
// }

inline Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return static_cast<Quaternion>(rtm::quat_lerp(q1.m_Q, q2.m_Q, t));
}

inline Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept
{
    return static_cast<Quaternion>(rtm::quat_slerp(q1.m_Q, q2.m_Q, t));
}


inline Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept
{
    return static_cast<Quaternion>(rtm::quat_mul(q1.m_Q, q2.m_Q));
}

// inline float Quaternion::Angle(const Quaternion& q1, const Quaternion& q2) noexcept
// {
//     throw std::runtime_error("Not implemented");
// }
