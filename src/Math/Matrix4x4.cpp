#include "Matrix4x4.h"

#include "Vector3.h"

namespace gore
{

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

} // namespace gore