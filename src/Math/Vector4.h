#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/vector4f.h"

namespace gore
{

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