#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/quatf.h"

namespace gore
{

ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Vector4);

ENGINE_STRUCT(Quaternion)
{
public:
    float x;
    float y;
    float z;
    float w;

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

public:
    // Common Values
    static const Quaternion Identity;
};

MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(Quaternion);

} // namespace gore