#pragma once

//------------------------------------------------------------------------------
// Comparison operators
//------------------------------------------------------------------------------

// TODO

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Matrix4x4 operator*(const Matrix4x4& V1, const Matrix4x4& V2) noexcept
{
    using namespace rtm;
    return static_cast<Matrix4x4>(matrix_mul(static_cast<Matrix4x4::SIMDValueType>(V1), static_cast<Matrix4x4::SIMDValueType>(V2)));
}

//------------------------------------------------------------------------------
// Matrix operations & Static functions
//------------------------------------------------------------------------------

// TODO