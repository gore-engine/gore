#pragma once

#include <ostream>

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/quatf.h"

namespace gore
{

ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Vector4);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Quaternion)
{
public:
    float x;
    float y;
    float z;
    float w;

    friend std::ostream& operator<<(std::ostream& os, const Quaternion& q) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::quatf);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Quaternion);

    SHALLOW_COPYABLE(Quaternion);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Quaternion);
    MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(Quaternion);
    MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Quaternion);

    Quaternion() noexcept :
        x(0),
        y(0),
        z(0),
        w(1)
    {
    }
    constexpr Quaternion(float ix, float iy, float iz, float iw) noexcept :
        x(ix),
        y(iy),
        z(iz),
        w(iw)
    {
    }
    Quaternion(const Vector3& v, float scalar) noexcept;
    explicit Quaternion(const Vector4& v) noexcept;
    explicit Quaternion(const float* pArray) noexcept;

    // Quaternion operations
    float Length() const noexcept;
    float LengthSquared() const noexcept;

    void Normalize() noexcept;
    void Normalize(Quaternion& result) const noexcept;

    void Conjugate() noexcept;
    void Conjugate(Quaternion& result) const noexcept;

    void Inverse(Quaternion& result) const noexcept;

    float Dot(const Quaternion& Q) const noexcept;

    void RotateTowards(const Quaternion& target, float maxAngle) noexcept;
    void DECLTYPE RotateTowards(const Quaternion& target, float maxAngle, Quaternion& result) const noexcept;

    // Computes rotation about y-axis (y), then x-axis (x), then z-axis (z)
    Vector3 ToEuler() const noexcept;

    // Static functions
    static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;

    // Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
    static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

    // Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
    static Quaternion CreateFromYawPitchRoll(const Vector3& angles) noexcept;

    static Quaternion CreateFromRotationMatrix(const Matrix4x4& M) noexcept;

    static void Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
    static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

    static void Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result) noexcept;
    static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) noexcept;

    static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result) noexcept;
    static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2) noexcept;

    static void DECLTYPE FromToRotation(const Vector3& fromDir, const Vector3& toDir, Quaternion& result) noexcept;
    static Quaternion FromToRotation(const Vector3& fromDir, const Vector3& toDir) noexcept;

    static void DECLTYPE LookRotation(const Vector3& forward, const Vector3& up, Quaternion& result) noexcept;
    static Quaternion LookRotation(const Vector3& forward, const Vector3& up) noexcept;

    static float Angle(const Quaternion& q1, const Quaternion& q2) noexcept;

public:
    // Common Values
    static const Quaternion Identity;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Quaternion);

} // namespace gore