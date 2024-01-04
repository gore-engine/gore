#include "Vector4.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Vector4& v) noexcept
{
    return os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
}

} // namespace gore