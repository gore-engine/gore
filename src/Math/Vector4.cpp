#include "Vector4.h"

#include "rtm/vector4f.h"

namespace gore
{

using namespace rtm;

std::ostream& operator<<(std::ostream& os, const Vector4& v) noexcept
{
    return os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

} // namespace gore