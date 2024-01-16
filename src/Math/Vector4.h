#pragma once

#include <ostream>
#include "Export.h"

#include "Math/Defines.h"

#include "rtm/vector4f.h"

namespace gore
{

struct Vector2;
struct Vector3;
struct Quaternion;
struct Matrix4x4;

ENGINE_STRUCT(Vector4)
{
public:
    float x;
    float y;
    float z;
    float w;

    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream & os, const Vector4& v) noexcept;

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
    [[nodiscard]] bool InBounds(const Vector4& Bounds) const noexcept;

    [[nodiscard]] float Length() const noexcept;
    [[nodiscard]] float LengthSquared() const noexcept;

    [[nodiscard]] float Dot(const Vector4& V) const noexcept;
    [[nodiscard]] static float Dot(const Vector4& lhs, const Vector4& rhs) noexcept;

    [[nodiscard]] Vector4 Normalized() const noexcept;
    void Normalize() noexcept;
    static void Normalize(Vector4 & v) noexcept;

    [[nodiscard]] static Vector4 Clamp(const Vector4& v, const Vector4& vmin, const Vector4& vmax) noexcept;

    // Static functions
    [[nodiscard]] static float Distance(const Vector4& v1, const Vector4& v2) noexcept;
    [[nodiscard]] static float DistanceSquared(const Vector4& v1, const Vector4& v2) noexcept;

    [[nodiscard]] static Vector4 Min(const Vector4& v1, const Vector4& v2) noexcept;
    [[nodiscard]] static Vector4 Max(const Vector4& v1, const Vector4& v2) noexcept;

    [[nodiscard]] static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t) noexcept;
    [[nodiscard]] static Vector4 SmoothStep(const Vector4& v1, const Vector4& v2, float t) noexcept;

    [[nodiscard]] static Vector4 Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g) noexcept;
    [[nodiscard]] static Vector4 CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t) noexcept;
    [[nodiscard]] static Vector4 Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t) noexcept;

    [[nodiscard]] static Vector4 Reflect(const Vector4& ivec, const Vector4& nvec) noexcept;

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

#include "Vector4.inl"

} // namespace gore