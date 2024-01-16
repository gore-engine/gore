#pragma once

// Conversion with SIMDValueType
Matrix4x4::operator SIMDValueType() const noexcept
{
    return m_M;
}

Matrix4x4::Matrix4x4(const Matrix4x4::SIMDValueType& F) noexcept :
    m_M(F)
{
}

Matrix4x4::Matrix4x4(Matrix4x4::SIMDValueType&& F) noexcept :
    m_M(std::move(F))
{
}

Matrix4x4& Matrix4x4::operator=(const Matrix4x4::SIMDValueType& F) noexcept
{
    m_M = F;
    return *this;
}

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