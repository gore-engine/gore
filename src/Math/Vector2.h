#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/types.h"
#include "rtm/vector4f.h"
#include "rtm/impl/vector_common.h"

namespace gore
{

ENGINE_STRUCT(Vector4);
ENGINE_STRUCT(Quaternion);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Vector2)
{
public:
    float x;
    float y;

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

public:
    // Common Values
    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 UnitX;
    static const Vector2 UnitY;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Vector2);

} // namespace gore