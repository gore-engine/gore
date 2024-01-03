#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "rtm/quatf.h"

std::ostream& operator<<(std::ostream& os, const Plane& p) noexcept
{
    return os << "Plane(" << p.x << ", " << p.y << ", " << p.z << ", " << p.w << ")";
}

inline Plane::Plane(const Vector3& normal, float d) noexcept :
    x(normal.x),
    y(normal.y),
    z(normal.z),
    w(d)
{
}


inline Plane::Plane(const Vector4& v) noexcept :
    x(v.x),
    y(v.y),
    z(v.z),
    w(v.w)
{
}

inline Plane::Plane(const float* pArray) noexcept :
    x(pArray[0]),
    y(pArray[1]),
    z(pArray[2]),
    w(pArray[3])
{
}

inline Plane& Plane::operator=(SIMDValueType&& F) noexcept
{
    rtm::quat_store(F, reinterpret_cast<float*>(this));
    return *this;
}

inline Vector3 Plane::Normal() const noexcept
{
    return Vector3(x, y, z);
}

inline void Plane::Normal(const Vector3& normal) noexcept
{
    x = normal.x;
    y = normal.y;
    z = normal.z;
}
