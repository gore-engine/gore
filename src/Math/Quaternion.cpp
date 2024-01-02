#include "Quaternion.h"

namespace gore
{

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