#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/matrix4x4f.h"

namespace gore
{

ENGINE_STRUCT(Matrix4x4)
{
public:
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::matrix4x4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Matrix4x4);

    SHALLOW_COPYABLE(Matrix4x4);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Matrix4x4);

    Matrix4x4() noexcept = default;

public:
    // Common Values
    static const Matrix4x4 Identity;
};

MATHF_MATRIX_BINARY_OPERATOR_DECLARATIONS(Matrix4x4);

} // namespace gore