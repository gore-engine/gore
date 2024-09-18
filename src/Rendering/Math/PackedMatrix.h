#pragma once

#include "Prefix.h"

#include "Math/Matrix4x4.h"

namespace gore::renderer
{
struct PackedMatrix
{
public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::matrix3x4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(PackedMatrix);

    SHALLOW_COPYABLE(PackedMatrix);

    MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(PackedMatrix);

    PackedMatrix() noexcept = default;

    explicit PackedMatrix(Matrix4x4 matrix) noexcept :
        m_M(rtm::matrix3x4f(
            matrix.m_M.x_axis,
            matrix.m_M.y_axis,
            matrix.m_M.z_axis))
    {
    }

    SIMDValueType m_M;
};
} // namespace gore::renderer