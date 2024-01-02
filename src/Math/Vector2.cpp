#include "Vector2.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Vector2& v) noexcept
{
    return os << "Vector2(" << v.x << ", " << v.y << ")";
}

} // namespace gore