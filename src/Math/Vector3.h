#pragma once

#include <ostream>

#include "Export.h"

#include "Math/Defines.h"
#include "Math/Quaternion.h"
#include "Math/Matrix4x4.h"

#include "rtm/matrix4x4f.h"
#include "rtm/vector4f.h"
#include "rtm/quatf.h"

namespace gore
{

ENGINE_STRUCT(Vector4);
ENGINE_STRUCT(Quaternion);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Vector3)
{
public:
    float x;
    float y;
    float z;

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::vector4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Vector3);

    SHALLOW_COPYABLE(Vector3);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Vector3);
    MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(Vector3);
    MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Vector3);


    Vector3() noexcept = default;
    constexpr explicit Vector3(float ix) noexcept :
        x(ix),
        y(ix),
        z(ix)
    {
    }
    constexpr explicit Vector3(float ix, float iy, float iz) noexcept :
        x(ix),
        y(iy),
        z(iz)
    {
    }

    // Vector operations
    bool InBounds(const Vector3& Bounds) const noexcept;

    float Length() const noexcept;
    float LengthSquared() const noexcept;

    float Dot(const Vector3& V) const noexcept;
    void Cross(const Vector3& V, Vector3& result) const noexcept;
    Vector3 Cross(const Vector3& V) const noexcept;

    void Normalize() noexcept;
    void Normalize(Vector3 & result) const noexcept;

    void Clamp(const Vector3& vmin, const Vector3& vmax) noexcept;
    void Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const noexcept;

    // Static functions
    static float Distance(const Vector3& v1, const Vector3& v2) noexcept;
    static float DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept;

    static void Min(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept;
    static Vector3 Min(const Vector3& v1, const Vector3& v2) noexcept;

    static void Max(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept;
    static Vector3 Max(const Vector3& v1, const Vector3& v2) noexcept;

    static void Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result) noexcept;
    static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) noexcept;

    static void SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result) noexcept;
    static Vector3 SmoothStep(const Vector3& v1, const Vector3& v2, float t) noexcept;

    static void Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g, Vector3& result) noexcept;
    static Vector3 Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g) noexcept;

    static void CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t, Vector3& result) noexcept;
    static Vector3 CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t) noexcept;

    static void Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result) noexcept;
    static Vector3 Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t) noexcept;

    static void Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result) noexcept;
    static Vector3 Reflect(const Vector3& ivec, const Vector3& nvec) noexcept;

    static void Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result) noexcept;
    static Vector3 Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex) noexcept;


public:
    // Common Values
    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;

    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Right;
    static const Vector3 Left;
    static const Vector3 Forward;
    static const Vector3 Backward;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Vector3);

#include "Vector3.inl"

} // namespace gore