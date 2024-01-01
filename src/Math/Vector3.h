#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/vector4f.h"

namespace gore
{

ENGINE_STRUCT(Vector3)
{
public:
    float x;
    float y;
    float z;

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

} // namespace gore