#include "Matrix4x4.h"

#include <type_traits>
#include <cmath>

#include "Vector3.h"
#include "Vector4.h"

#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "rtm/impl/matrix_common.h"
#include "rtm/impl/matrix_cast.h"

namespace gore
{

using namespace rtm;

std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) noexcept
{
    return os << "Matrix4x4()";
}

Matrix4x4::operator SIMDValueType() const noexcept
{
    return m_M;
}

Matrix4x4::Matrix4x4(gore::Matrix4x4::SIMDValueType F) noexcept
{
    m_M = F;
}

Matrix4x4::Matrix4x4(rtm::matrix3x4f&& F) noexcept :
    m_M(matrix_set(
        matrix_get_axis(F, rtm::axis4::x),
        matrix_get_axis(F, rtm::axis4::y),
        matrix_get_axis(F, rtm::axis4::z),
        vector_set(0.0f, 0.0f, 0.0f, 1.0f)))
{
}

// Properties
Vector3 Matrix4x4::Up() const noexcept
{
    return matrix_get_axis(m_M, rtm::axis4::y);
}
void Matrix4x4::Up(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::y);
}

Vector3 Matrix4x4::Down() const noexcept
{
    return -Up();
}
void Matrix4x4::Down(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::y);
}

Vector3 Matrix4x4::Right() const noexcept
{
    return matrix_get_axis(m_M, rtm::axis4::x);
}
void Matrix4x4::Right(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::x);
}

Vector3 Matrix4x4::Left() const noexcept
{
    return -Right();
}
void Matrix4x4::Left(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::x);
}

Vector3 Matrix4x4::Forward() const noexcept
{
    return matrix_get_axis(m_M, rtm::axis4::z);
}
void Matrix4x4::Forward(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::z);
}

Vector3 Matrix4x4::Backward() const noexcept
{
    return -Forward();
}
void Matrix4x4::Backward(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::z);
}

Vector3 Matrix4x4::Translation() const noexcept
{
    return matrix_get_axis(m_M, rtm::axis4::w);
}
void Matrix4x4::Translation(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, static_cast<Vector4::SIMDValueType>(v.AsPoint()), rtm::axis4::w);
}

Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& position) noexcept
{
    return rtm::matrix_from_translation(static_cast<Vector4::SIMDValueType>(position.AsPoint()));
}

Matrix4x4 Matrix4x4::CreateTranslation(float x, float y, float z) noexcept
{
    return rtm::matrix_from_translation(rtm::vector_set(x, y, z, 1.0f));
}

Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfViewLH(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept
{
    float SinFov = sinf(fov / 2);
    float CosFov = cosf(fov / 2);
    float Height = CosFov / SinFov;
    float Width  = Height / aspectRatio;
    float fRange = farPlane / (farPlane - nearPlane);

    SIMDValueType m = matrix_set(
        vector_set(Width, 0.0f, 0.0f, 0.0f),
        vector_set(0.0f, Height, 0.0f, 0.0f),
        vector_set(0.0f, 0.0f, fRange, 1.0f),
        vector_set(0.0f, 0.0f, -fRange * nearPlane, 0.0f));
    return static_cast<Matrix4x4>(m);
}

} // namespace gore