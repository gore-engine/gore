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
    Quaternion::ValueType q2conj = rtm::vector_div(
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
