#include "Matrix4x4.h"

#include <type_traits>
#include <limits>
#include <cmath>

#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"

#include "rtm/impl/matrix_cast.h"
#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "rtm/impl/matrix_common.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) noexcept
{
    return os << "Matrix4x4()";
}

// Properties
Vector3 Matrix4x4::GetUp() const noexcept
{
    return static_cast<Vector3>(matrix_get_axis(m_M, rtm::axis4::y));
}
void Matrix4x4::SetUp(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::y);
}

Vector3 Matrix4x4::GetDown() const noexcept
{
    return -GetUp();
}
void Matrix4x4::SetDown(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::y);
}

Vector3 Matrix4x4::GetRight() const noexcept
{
    return static_cast<Vector3>(matrix_get_axis(m_M, rtm::axis4::x));
}
void Matrix4x4::SetRight(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::x);
}

Vector3 Matrix4x4::GetLeft() const noexcept
{
    return -GetRight();
}
void Matrix4x4::SetLeft(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::x);
}

Vector3 Matrix4x4::GetForward() const noexcept
{
    return static_cast<Vector3>(matrix_get_axis(m_M, rtm::axis4::z));
}
void Matrix4x4::SetForward(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, v, rtm::axis4::z);
}

Vector3 Matrix4x4::GetBackward() const noexcept
{
    return -GetForward();
}
void Matrix4x4::SetBackward(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, -v, rtm::axis4::z);
}

Vector3 Matrix4x4::GetTranslation() const noexcept
{
    return static_cast<Vector3>(matrix_get_axis(m_M, rtm::axis4::w));
}
void Matrix4x4::SetTranslation(const Vector3& v) noexcept
{
    m_M = matrix_set_axis(m_M, static_cast<Vector4::SIMDValueType>(v.AsPoint()), rtm::axis4::w);
}

Quaternion Matrix4x4::GetRotation() const noexcept
{
    return static_cast<Quaternion>(quat_from_matrix(rtm::matrix3x4f(rtm::matrix_cast(m_M))));
}

Matrix4x4 Matrix4x4::FromTRS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qvv(static_cast<Quaternion::SIMDValueType>(rotation), static_cast<Vector3::SIMDValueType>(translation), static_cast<Vector3::SIMDValueType>(scale)));
}

Matrix4x4 Matrix4x4::FromTRNoScale(const Vector3& translation, const Quaternion& rotation) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_qv(static_cast<Quaternion::SIMDValueType>(rotation), static_cast<Vector3::SIMDValueType>(translation)));
}

Matrix4x4 Matrix4x4::FromTranslation(const Vector3& position) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_translation(static_cast<Vector4::SIMDValueType>(position.AsPoint())));
}

Matrix4x4 Matrix4x4::FromTranslation(float x, float y, float z) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix_from_translation(rtm::vector_set(x, y, z, 1.0f)));
}

Matrix4x4 Matrix4x4::FromAxisAngle(const Vector3& axis, float angle) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix3x4f(rtm::matrix_from_quat(rtm::quat_from_axis_angle(static_cast<Vector3::SIMDValueType>(axis), angle))));
}

Matrix4x4 Matrix4x4::FromQuaternion(const Quaternion& quat) noexcept
{
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, rtm::matrix3x4f(rtm::matrix_from_quat(static_cast<Quaternion::SIMDValueType>(quat))));
}

Matrix4x4 Matrix4x4::Transpose() const noexcept
{
    return static_cast<Matrix4x4>(matrix_transpose(static_cast<Matrix4x4::SIMDValueType>(m_M)));
}

void Matrix4x4::Transpose(Matrix4x4& m) noexcept
{
    m.m_M = matrix_transpose(static_cast<Matrix4x4::SIMDValueType>(m.m_M));
}

Matrix4x4 Matrix4x4::Inverse() const noexcept
{
    return static_cast<Matrix4x4>(matrix_inverse((m_M)));
}

void Matrix4x4::Invert() noexcept
{
    m_M = matrix_inverse((m_M));
}

void Matrix4x4::Invert(Matrix4x4& m) noexcept
{
    m.m_M = matrix_inverse((m.m_M));
}

Matrix4x4 Matrix4x4::CreatePerspectiveFieldOfViewLH(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept
{
    float SinFov = sinf(fov / 2);
    float CosFov = cosf(fov / 2);
    float Height = CosFov / SinFov;
    float Width  = Height / aspectRatio;
    float a, b;
    if (std::isinf(farPlane))
    {
        a = 0.0f;
        b = nearPlane;
    }
    else
    {
        a = -nearPlane / (farPlane - nearPlane);
        b = -farPlane * a;
    }

    return static_cast<Matrix4x4>(SIMDValueType(rtm::matrix_set(
        rtm::vector_set(Width, 0.0f, 0.0f, 0.0f),
        rtm::vector_set(0.0f, Height, 0.0f, 0.0f),
        rtm::vector_set(0.0f, 0.0f, a, 1.0f),
        rtm::vector_set(0.0f, 0.0f, b, 0.0f))));
}
Matrix4x4 Matrix4x4::CreateOrthographicLH(float width, float height, float nearPlane, float farPlane) noexcept
{
    float fRange = 1.0f / (farPlane - nearPlane);

    return static_cast<Matrix4x4>(rtm::matrix_set(
        rtm::vector_set(2.0f / width, 0.0f, 0.0f, 0.0f),
        rtm::vector_set(0.0f, 2.0f / height, 0.0f, 0.0f),
        rtm::vector_set(0.0f, 0.0f, -fRange, 0.0f),
        rtm::vector_set(0.0f, 0.0f, fRange * farPlane, 1.0f)));
}

} // namespace gore