#include "Vector3.h"

#include "Vector4.h"

#include "rtm/vector4f.h"

namespace gore
{

using namespace rtm;

std::ostream& operator<<(std::ostream& os, const Vector3& v) noexcept
{
    return os << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

Vector3::operator SIMDValueType() const noexcept
{
    return vector_load3((reinterpret_cast<const float*>(this)));
}

Vector3::Vector3(SIMDValueType F) noexcept :
    x(),
    y(),
    z()
{
    vector_store(F, reinterpret_cast<float*>(this));
}

Vector4 Vector3::AsPoint() const noexcept
{
    return Vector4(x, y, z, 1.0f);
}

Vector4 Vector3::AsVector() const noexcept
{
    return Vector4(x, y, z, 0.0f);
}

} // namespace gore