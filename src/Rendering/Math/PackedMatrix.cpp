#include "PackedMatrix.h"

namespace gore::renderer
{
bool PackedMatrix::operator==(const PackedMatrix& other) const noexcept
{
    return rtm::vector_all_near_equal3(m_M.x_axis, other.m_M.x_axis) && rtm::vector_all_near_equal3(m_M.y_axis, other.m_M.y_axis) && rtm::vector_all_near_equal3(m_M.z_axis, other.m_M.z_axis);
}

bool PackedMatrix::operator!=(const PackedMatrix& other) const noexcept
{
    return !(*this == other);
}

} // namespace gore::renderer