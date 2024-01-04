#include "Matrix4x4.h"

#include <type_traits>

#include "Vector3.h"

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
    return matrix_set(vector_load(m[0]), vector_load(m[1]), vector_load(m[2]), vector_load(m[3]));
}

Matrix4x4::Matrix4x4(gore::Matrix4x4::SIMDValueType F) noexcept
{
    vector_store(matrix_get_axis(F, rtm::axis4::x), m[0]);
    vector_store(matrix_get_axis(F, rtm::axis4::y), m[1]);
    vector_store(matrix_get_axis(F, rtm::axis4::z), m[2]);
    vector_store(matrix_get_axis(F, rtm::axis4::w), m[3]);
}

Matrix4x4::Matrix4x4(rtm::matrix3x4f&& F) noexcept :
    _41(0),
    _42(0),
    _43(0),
    _44(1)
{
    vector_store(matrix_get_axis(F, rtm::axis4::x), m[0]);
    vector_store(matrix_get_axis(F, rtm::axis4::y), m[1]);
    vector_store(matrix_get_axis(F, rtm::axis4::z), m[2]);
}

// Properties
Vector3 Matrix4x4::Up() const noexcept
{
    return Vector3(_21, _22, _23);
}
void Matrix4x4::Up(const Vector3& v) noexcept
{
    _21 = v.x;
    _22 = v.y;
    _23 = v.z;
}

Vector3 Matrix4x4::Down() const noexcept
{
    return Vector3(-_21, -_22, -_23);
}
void Matrix4x4::Down(const Vector3& v) noexcept
{
    _21 = -v.x;
    _22 = -v.y;
    _23 = -v.z;
}

Vector3 Matrix4x4::Right() const noexcept
{
    return Vector3(_11, _12, _13);
}
void Matrix4x4::Right(const Vector3& v) noexcept
{
    _11 = v.x;
    _12 = v.y;
    _13 = v.z;
}

Vector3 Matrix4x4::Left() const noexcept
{
    return Vector3(-_11, -_12, -_13);
}
void Matrix4x4::Left(const Vector3& v) noexcept
{
    _11 = -v.x;
    _12 = -v.y;
    _13 = -v.z;
}

Vector3 Matrix4x4::Forward() const noexcept
{
    return Vector3(-_31, -_32, -_33);
}
void Matrix4x4::Forward(const Vector3& v) noexcept
{
    _31 = -v.x;
    _32 = -v.y;
    _33 = -v.z;
}

Vector3 Matrix4x4::Backward() const noexcept
{
    return Vector3(_31, _32, _33);
}
void Matrix4x4::Backward(const Vector3& v) noexcept
{
    _31 = v.x;
    _32 = v.y;
    _33 = v.z;
}

Vector3 Matrix4x4::Translation() const noexcept
{
    return Vector3(_41, _42, _43);
}
void Matrix4x4::Translation(const Vector3& v) noexcept
{
    _41 = v.x;
    _42 = v.y;
    _43 = v.z;
}

inline Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& position) noexcept
{
    return rtm::matrix_from_translation(position);
}

inline Matrix4x4 Matrix4x4::CreateTranslation(float x, float y, float z) noexcept
{
    return rtm::matrix_from_translation(vector_set(x, y, z));
}

} // namespace gore