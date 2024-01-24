#include "TQS.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const TQS& tqs) noexcept
{
    os << "TQS(" << tqs.t << ", " << tqs.q << ", " << tqs.s << ")";
    return os;
}

} // namespace gore