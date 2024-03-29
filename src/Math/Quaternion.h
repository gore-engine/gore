#pragma once

#include <ostream>

#include "Export.h"

#include "Math/Defines.h"
#include "Math/Vector3.h"

#include "rtm/vector4f.h"
#include "rtm/quatf.h"
#include "rtm/impl/quat_common.h"

namespace gore
{

struct Vector3;
struct Matrix4x4;

ENGINE_STRUCT(Quaternion)
{
public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::quatf);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Quaternion);

    SHALLOW_COPYABLE(Quaternion);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Quaternion);
    MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(Quaternion);
    MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Quaternion);

public:
    SIMDValueType m_Q;
    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream & os, const Quaternion& q) noexcept;

    Quaternion() noexcept :
        m_Q(Identity)
    {
    }
    Quaternion(float ix, float iy, float iz, float iw) noexcept :
        m_Q(rtm::vector_to_quat(rtm::vector_set(ix, iy, iz, iw)))
    {
    }

    Quaternion(const Vector3& v, float scalar) noexcept;
    explicit Quaternion(const float* pArray) noexcept;

    // Quaternion operations
    [[nodiscard]] float Length() const noexcept;
    [[nodiscard]] float LengthSquared() const noexcept;

    [[nodiscard]] bool IsNormalized() const noexcept;
    [[nodiscard]] Quaternion Normalized() const noexcept;
    void Normalize() noexcept;
    static void Normalize(Quaternion & q) noexcept;

    [[nodiscard]] Quaternion Conjucated() const noexcept;
    void Conjugate() noexcept;
    static void Conjugate(Quaternion & q) noexcept;

    // I don't think it's necessary to divide by length squared in this engine
    // Nobody will really use a non-normalized quaternion and treat it as a regular rotation
    // So the function here in this engine is effectively the same as Conjugate()
    [[nodiscard]] Quaternion Inverse() const noexcept;
    void Invert() noexcept;
    static void Invert(Quaternion & q) noexcept;

    [[nodiscard]] float Dot(const Quaternion& Q) const noexcept;

    // Computes rotation about y-axis (y), then x-axis (x), then z-axis (z)
    [[nodiscard]] Vector3 ToEuler() const noexcept;

    // Static functions
    [[nodiscard]] static Quaternion FromAxisAngle(const Vector3& axis, float angle) noexcept;
    // Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
    [[nodiscard]] static Quaternion FromYawPitchRoll(float yaw, float pitch, float roll) noexcept;
    // Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
    [[nodiscard]] static Quaternion FromYawPitchRoll(const Vector3& angles) noexcept;
    [[nodiscard]] static Quaternion FromRotationMatrix(const Matrix4x4& M) noexcept;

    [[nodiscard]] static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;
    [[nodiscard]] static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;
    [[nodiscard]] static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept;
    [[nodiscard]] static Quaternion FromToRotation(const Vector3& fromDir, const Vector3& toDir) noexcept;
    [[nodiscard]] static Quaternion LookRotation(const Vector3& forward, const Vector3& up) noexcept;

    [[nodiscard]] static float Angle(const Quaternion& q1, const Quaternion& q2) noexcept;

public:
    // Common Values
    static const Quaternion Identity;
};

Quaternion operator+(const Quaternion& Q1, const Quaternion& Q2) noexcept;
Quaternion operator-(const Quaternion& Q1, const Quaternion& Q2) noexcept;
Quaternion operator*(const Quaternion& Q1, const Quaternion& Q2) noexcept;
Quaternion operator*(const Quaternion& Q, float S) noexcept;
Quaternion operator/(const Quaternion& Q1, const Quaternion& Q2) noexcept;
Quaternion operator*(float S, const Quaternion& Q) noexcept;

#include "Math/Quaternion.inl"

} // namespace gore