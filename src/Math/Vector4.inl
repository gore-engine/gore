Vector4::operator SIMDValueType() const noexcept
{
    return rtm::vector_load((reinterpret_cast<const float*>(this)));
}