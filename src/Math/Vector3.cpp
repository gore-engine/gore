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

Vector3::Vector3(const Vector3::SIMDValueType& F) noexcept :
    x(rtm::vector_get_x(F)),
    y(rtm::vector_get_y(F)),
    z(rtm::vector_get_z(F))
{
}

Vector3::Vector3(Vector3::SIMDValueType&& F) noexcept :
    x(rtm::vector_get_x(std::move(F))),
    y(rtm::vector_get_y(std::move(F))),
    z(rtm::vector_get_z(std::move(F)))
{
}

Vector3& Vector3::operator=(const Vector3::SIMDValueType& F) noexcept
{
    rtm::vector_store3(F, reinterpret_cast<float*>(this));
    return *this;
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