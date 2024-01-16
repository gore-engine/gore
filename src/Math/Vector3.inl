#pragma once

// Conversion with SIMDValueType
Vector3::operator SIMDValueType() const noexcept
{
    using namespace rtm;
    return vector_load3((reinterpret_cast<const float*>(this)));
}

Vector3::Vector3(const Vector3::SIMDValueType& F) noexcept :
    x(rtm::vector_get_x(F)),
    y(rtm::vector_get_y(F)),
    z(rtm::vector_get_z(F))
{
}

Vector3::Vector3(Vector3::SIMDValueType&& F) noexcept :
    x(rtm::vector_get_x(std::move(F))),
    y(rtm::vector_get_y(std::move(F))),
    z(rtm::vector_get_z(std::move(F)))
{
}

Vector3& Vector3::operator=(const Vector3::SIMDValueType& F) noexcept
{
    rtm::vector_store3(F, reinterpret_cast<float*>(this));
    return *this;
}

//------------------------------------------------------------------------------
// Comparison operators
//------------------------------------------------------------------------------

inline bool Vector3::operator==(const Vector3& V) const noexcept
{
    using namespace rtm;
    return vector_all_equal3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

inline bool Vector3::operator!=(const Vector3& V) const noexcept
{
    using namespace rtm;
    return !vector_all_equal3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Vector3& Vector3::operator+=(const Vector3& V) noexcept
{
    using namespace rtm;
    *this = vector_add(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
    return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& V) noexcept
{
    using namespace rtm;
    *this = vector_sub(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
    return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& V) noexcept
{
    using namespace rtm;
    *this = vector_mul(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
    return *this;
}

inline Vector3& Vector3::operator*=(float S) noexcept
{
    using namespace rtm;
    *this = vector_mul(static_cast<SIMDValueType>(*this), S);
    return *this;
}

inline Vector3& Vector3::operator/=(float S) noexcept
{
    using namespace rtm;
    *this = vector_div(static_cast<SIMDValueType>(*this), vector_set(S));
    return *this;
}

//------------------------------------------------------------------------------
// Unary operators
//------------------------------------------------------------------------------

inline Vector3 Vector3::operator-() const noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_neg(static_cast<SIMDValueType>(*this)));
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Vector3 operator+(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_add(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2)));
}

inline Vector3 operator-(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_sub(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2)));
}

inline Vector3 operator*(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_mul(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2)));
}

inline Vector3 operator*(const Vector3& V, float S) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_mul(static_cast<Vector3::SIMDValueType>(V), S));
}

inline Vector3 operator/(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_div(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2)));
}

inline Vector3 operator/(const Vector3& V, float S) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_div(static_cast<Vector3::SIMDValueType>(V), vector_set(S)));
}

inline Vector3 operator*(float S, const Vector3& V) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_mul(static_cast<Vector3::SIMDValueType>(V), S));
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool Vector3::InBounds(const Vector3& Bounds) const noexcept
{
    using namespace rtm;
    return vector_all_less_equal3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(Bounds))
           && vector_all_greater_equal3(static_cast<SIMDValueType>(*this), vector_neg(static_cast<SIMDValueType>(Bounds)));
}

inline float Vector3::Length() const noexcept
{
    using namespace rtm;
    return vector_length3(static_cast<SIMDValueType>(*this));
}

inline float Vector3::LengthSquared() const noexcept
{
    using namespace rtm;
    return vector_length_squared3(static_cast<SIMDValueType>(*this));
}

inline float Vector3::Dot(const Vector3& V) const noexcept
{
    using namespace rtm;
    return vector_dot3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

float Vector3::Dot(const Vector3& lhs, const Vector3& rhs) noexcept
{
    using namespace rtm;
    return vector_dot3(static_cast<SIMDValueType>(lhs), static_cast<SIMDValueType>(rhs));
}

inline Vector3 Vector3::Cross(const Vector3& rhs) const noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_cross3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(rhs)));
}

inline Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_cross3(static_cast<SIMDValueType>(lhs), static_cast<SIMDValueType>(rhs)));
}

inline Vector3 Vector3::Normalized() const noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_normalize3(static_cast<SIMDValueType>(*this)));
}

inline void Vector3::Normalize() noexcept
{
    using namespace rtm;
    *this = vector_normalize3(static_cast<SIMDValueType>(*this));
}

inline void Vector3::Normalize(Vector3& v) noexcept
{
    using namespace rtm;
    v = vector_normalize3(static_cast<SIMDValueType>(v));
}

inline void Vector3::Clamp(const Vector3& vMin, const Vector3& vMax) noexcept
{
    using namespace rtm;
    *this = vector_clamp(
        static_cast<SIMDValueType>(*this),
        static_cast<SIMDValueType>(vMin),
        static_cast<SIMDValueType>(vMax));
}

inline Vector3 Vector3::Clamp(const Vector3& input, const Vector3& vMin, const Vector3& vMax) const noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_clamp(
        static_cast<SIMDValueType>(input),
        static_cast<SIMDValueType>(vMin),
        static_cast<SIMDValueType>(vMax)));
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float Vector3::Distance(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_length3(vector_sub(static_cast<SIMDValueType>(v2), static_cast<SIMDValueType>(v1)));
}

inline float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_length_squared3(vector_sub(static_cast<SIMDValueType>(v2), static_cast<SIMDValueType>(v1)));
}

inline Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_min(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2)));
}

inline Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_max(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2)));
}

inline Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t) noexcept
{
    using namespace rtm;
    return static_cast<Vector3>(vector_lerp(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2), t));
}

inline Vector3 Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t) noexcept
{
    using namespace rtm;
    t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t); // Clamp value to 0 to 1
    t = t * t * (3.f - 2.f * t);
    return static_cast<Vector3>(vector_lerp(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2), t));
}

// inline Vector3 Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline Vector3 Vector3::Reflect(const Vector3& ivec, const Vector3& nvec) noexcept
//{
//     throw std::exception("Not implemented");
// }
//