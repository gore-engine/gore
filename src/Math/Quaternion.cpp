#include "Quaternion.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Quaternion& q) noexcept
{
    return os << "Quaternion(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
}

Quaternion::operator ValueType() const noexcept
{
    return rtm::quat_load(reinterpret_cast<const float*>(this));
}

Quaternion::Quaternion(ValueType&& F) noexcept :
    Quaternion()
{
    rtm::quat_store(F, reinterpret_cast<float*>(this));
}

Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept
{
    return Quaternion(rtm::quat_from_euler(pitch, yaw, roll));
}

} // namespace gore