#include "Vector3.h"

#include "rtm/vector4f.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Vector3& v) noexcept
{
    return os << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

Vector3::operator ValueType() const noexcept
{
    return rtm::vector_load3((reinterpret_cast<const float*>(this)));
}

} // namespace gore