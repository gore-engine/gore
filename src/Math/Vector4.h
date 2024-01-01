#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/vector4f.h"

namespace gore
{

ENGINE_STRUCT(Vector2);
ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Quaternion);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Vector4)
{
public:
    float x;
    float y;
    float z;
    float w;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::vector4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Vector4);

    SHALLOW_COPYABLE(Vector4);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Vector4);
    MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(Vector4);
    MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Vector4);


    Vector4() noexcept = default;
    constexpr explicit Vector4(float ix) noexcept :
        x(ix),
        y(ix),
        z(ix),
        w(ix)
    {
    }
    constexpr explicit Vector4(float ix, float iy, float iz, float iw) noexcept :
        x(ix),
        y(iy),
        z(iz),
        w(iw)
    {
    }


    // Vector operations
    bool InBounds(const Vector4& Bounds) const noexcept;

    float Length() const noexcept;
    float LengthSquared() const noexcept;

    float Dot(const Vector4& V) const noexcept;
    void Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const noexcept;
    Vector4 Cross(const Vector4& v1, const Vector4& v2) const noexcept;

    void Normalize() noexcept;
    void Normalize(Vector4& result) const noexcept;

    void Clamp(const Vector4& vmin, const Vector4& vmax) noexcept;
    void Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const noexcept;

    // Static functions
    static float Distance(const Vector4& v1, const Vector4& v2) noexcept;
    static float DistanceSquared(const Vector4& v1, const Vector4& v2) noexcept;

    static void Min(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
    static Vector4 Min(const Vector4& v1, const Vector4& v2) noexcept;

    static void Max(const Vector4& v1, const Vector4& v2, Vector4& result) noexcept;
    static Vector4 Max(const Vector4& v1, const Vector4& v2) noexcept;

    static void Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result) noexcept;
    static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t) noexcept;

    static void SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result) noexcept;
    static Vector4 SmoothStep(const Vector4& v1, const Vector4& v2, float t) noexcept;

    static void Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result) noexcept;
    static Vector4 Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g) noexcept;

    static void CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result) noexcept;
    static Vector4 CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t) noexcept;

    static void Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result) noexcept;
    static Vector4 Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t) noexcept;

    static void Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result) noexcept;
    static Vector4 Reflect(const Vector4& ivec, const Vector4& nvec) noexcept;

    static void Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result) noexcept;
    static Vector4 Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex) noexcept;

    static void Transform(const Vector2& v, const Quaternion& quat, Vector4& result) noexcept;
    static Vector4 Transform(const Vector2& v, const Quaternion& quat) noexcept;

    static void Transform(const Vector3& v, const Quaternion& quat, Vector4& result) noexcept;
    static Vector4 Transform(const Vector3& v, const Quaternion& quat) noexcept;

    static void Transform(const Vector4& v, const Quaternion& quat, Vector4& result) noexcept;
    static Vector4 Transform(const Vector4& v, const Quaternion& quat) noexcept;

    static void Transform(const Vector4& v, const Matrix4x4& m, Vector4& result) noexcept;
    static Vector4 Transform(const Vector4& v, const Matrix4x4& m) noexcept;
    static void Transform(const Vector4* varray, size_t count, const Matrix4x4& m, Vector4* resultArray) noexcept;


public:
    // Common Values
    static const Vector4 Zero;
    static const Vector4 One;
    static const Vector4 UnitX;
    static const Vector4 UnitY;
    static const Vector4 UnitZ;
    static const Vector4 UnitW;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Vector4);

} // namespace gore