#pragma once

Vector4::operator SIMDValueType() const noexcept
{
    return rtm::vector_load((reinterpret_cast<const float*>(this)));
}

Vector4::Vector4(const Vector4::SIMDValueType& F) noexcept :
    x(rtm::vector_get_x(F)),
    y(rtm::vector_get_y(F)),
    z(rtm::vector_get_z(F)),
    w(rtm::vector_get_w(F))
{
}

Vector4::Vector4(Vector4::SIMDValueType&& F) noexcept :
    x(rtm::vector_get_x(std::move(F))),
    y(rtm::vector_get_y(std::move(F))),
    z(rtm::vector_get_z(std::move(F))),
    w(rtm::vector_get_w(std::move(F)))
{
}