#include "Vector3.h"

#include "rtm/vector4f.h"

namespace gore
{

using namespace rtm;

std::ostream& operator<<(std::ostream& os, const Vector3& v) noexcept
{
    return os << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

Vector3::operator ValueType() const noexcept
{
    return vector_load3((reinterpret_cast<const float*>(this)));
}

Vector3::Vector3(ValueType F) noexcept :
    x(),
    y(),
    z()
{
    vector_store(F, reinterpret_cast<float*>(this));
}

} // namespace gore