#pragma once

#include <ostream>

#include "Export.h"

#include "Math/Defines.h"
#include "Math/Matrix4x4.h"

#include "rtm/matrix4x4f.h"
#include "rtm/vector4f.h"
#include "rtm/quatf.h"

namespace gore
{

struct Vector4;
struct Quaternion;

ENGINE_STRUCT(Vector3)
{
public:
    float x;
    float y;
    float z;

    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream & os, const Vector3& v) noexcept;

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
    [[nodiscard]] inline bool InBounds(const Vector3& Bounds) const noexcept;

    [[nodiscard]] inline float Length() const noexcept;
    [[nodiscard]] inline float LengthSquared() const noexcept;

    [[nodiscard]] inline float Dot(const Vector3& V) const noexcept;
    [[nodiscard]] inline static float Dot(const Vector3& lhs, const Vector3& rhs) noexcept;
    [[nodiscard]] inline Vector3 Cross(const Vector3& rhs) const noexcept;
    [[nodiscard]] inline static Vector3 Cross(const Vector3& lhs, const Vector3& rhs) noexcept;

    [[nodiscard]] inline Vector3 Normalized() const noexcept;
    inline void Normalize() noexcept;
    inline static void Normalize(Vector3 & v) noexcept;

    inline void Clamp(const Vector3& vMin, const Vector3& vMax) noexcept;
    [[nodiscard]] inline Vector3 Clamp(const Vector3& input, const Vector3& vMin, const Vector3& vMax) const noexcept;

    [[nodiscard]] Vector4 AsPoint() const noexcept;
    [[nodiscard]] Vector4 AsVector() const noexcept;

    // Static functions
    [[nodiscard]] inline static float Distance(const Vector3& v1, const Vector3& v2) noexcept;
    [[nodiscard]] inline static float DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept;

    [[nodiscard]] inline static Vector3 Min(const Vector3& v1, const Vector3& v2) noexcept;
    [[nodiscard]] inline static Vector3 Max(const Vector3& v1, const Vector3& v2) noexcept;

    [[nodiscard]] inline static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) noexcept;
    [[nodiscard]] inline static Vector3 SmoothStep(const Vector3& v1, const Vector3& v2, float t) noexcept;

    [[nodiscard]] inline static Vector3 Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g) noexcept;
    [[nodiscard]] inline static Vector3 CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t) noexcept;
    [[nodiscard]] inline static Vector3 Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t) noexcept;

    [[nodiscard]] inline static Vector3 Reflect(const Vector3& ivec, const Vector3& nvec) noexcept;

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

TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV_BEFORE_DEFINITION(TFrom, Vector3::SIMDValueType)
Vector3::Vector3(TFrom&& F) noexcept :
    x(rtm::vector_get_x(std::forward<SIMDValueType>(F))),
    y(rtm::vector_get_y(std::forward<SIMDValueType>(F))),
    z(rtm::vector_get_z(std::forward<SIMDValueType>(F)))
{
}

TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV_BEFORE_DEFINITION(TFrom, Vector3::SIMDValueType)
Vector3& Vector3::operator=(TFrom&& F) noexcept
{
    rtm::vector_store3(F, reinterpret_cast<float*>(this));
    return *this;
}

#include "Vector3.inl"

} // namespace gore