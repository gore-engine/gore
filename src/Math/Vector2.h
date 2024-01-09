#pragma once

#include <ostream>

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/types.h"
#include "rtm/vector4f.h"
#include "rtm/impl/vector_common.h"

namespace gore
{

struct Vector4;
struct Quaternion;
struct Matrix4x4;

ENGINE_STRUCT(Vector2)
{
public:
    float x;
    float y;

    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream& os, const Vector2& v) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::vector4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Vector2);

    SHALLOW_COPYABLE(Vector2);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Vector2);
    MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(Vector2);
    MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Vector2);


    Vector2() noexcept = default;
    constexpr explicit Vector2(float ix) noexcept :
        x(ix),
        y(ix)
    {
    }
    constexpr explicit Vector2(float ix, float iy) noexcept :
        x(ix),
        y(iy)
    {
    }

    // Vector operations
    [[nodiscard]] bool InBounds(const Vector2& Bounds) const noexcept;

    [[nodiscard]] float Length() const noexcept;
    [[nodiscard]] float LengthSquared() const noexcept;

    [[nodiscard]] float Dot(const Vector2& V) const noexcept;
    [[nodiscard]] static float Dot(const Vector2& lhs, const Vector2& rhs) noexcept;

    [[nodiscard]] Vector2 Normalized() noexcept;
    void Normalize() const noexcept;
    [[nodiscard]] static Vector2 Normalize(const Vector2& v) noexcept;

    [[nodiscard]] static Vector2 Clamp(const Vector2& v, const Vector2& vmin, const Vector2& vmax) noexcept;

    // Static functions
    [[nodiscard]] static float Distance(const Vector2& v1, const Vector2& v2) noexcept;
    [[nodiscard]] static float DistanceSquared(const Vector2& v1, const Vector2& v2) noexcept;

    [[nodiscard]] static Vector2 Min(const Vector2& v1, const Vector2& v2) noexcept;
    [[nodiscard]] static Vector2 Max(const Vector2& v1, const Vector2& v2) noexcept;

    [[nodiscard]] static Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t) noexcept;
    [[nodiscard]] static Vector2 SmoothStep(const Vector2& v1, const Vector2& v2, float t) noexcept;

    [[nodiscard]] static Vector2 Reflect(const Vector2& ivec, const Vector2& nvec) noexcept;

public:
    // Common Values
    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 UnitX;
    static const Vector2 UnitY;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Vector2);

} // namespace gore