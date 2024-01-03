//------------------------------------------------------------------------------
// Comparison operators
//------------------------------------------------------------------------------

inline bool Vector3::operator==(const Vector3& V) const noexcept
{
    return rtm::vector_all_equal3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

inline bool Vector3::operator!=(const Vector3& V) const noexcept
{
    return !rtm::vector_all_equal3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Vector3& Vector3::operator+=(const Vector3& V) noexcept
{
    using namespace rtm;
    vector_store(vector_add(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V)), reinterpret_cast<float*>(this));
    return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& V) noexcept
{
    using namespace rtm;
    vector_store(vector_sub(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V)), reinterpret_cast<float*>(this));
    return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& V) noexcept
{
    using namespace rtm;
    vector_store(vector_mul(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V)), reinterpret_cast<float*>(this));
    return *this;
}

inline Vector3& Vector3::operator*=(float S) noexcept
{
    using namespace rtm;
    vector_store(vector_mul(static_cast<SIMDValueType>(*this), S), reinterpret_cast<float*>(this));
    return *this;
}

inline Vector3& Vector3::operator/=(float S) noexcept
{
    using namespace rtm;
    vector_store(vector_div(static_cast<SIMDValueType>(*this), vector_set(S)), reinterpret_cast<float*>(this));
    return *this;
}

//------------------------------------------------------------------------------
// Unary operators
//------------------------------------------------------------------------------

inline Vector3 Vector3::operator-() const noexcept
{
    using namespace rtm;
    return Vector3(vector_neg(static_cast<SIMDValueType>(*this)));
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Vector3 operator+(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return vector_add(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2));
}

inline Vector3 operator-(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return vector_sub(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2));
}

inline Vector3 operator*(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return vector_mul(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2));
}

inline Vector3 operator*(const Vector3& V, float S) noexcept
{
    using namespace rtm;
    return vector_mul(static_cast<Vector3::SIMDValueType>(V), S);
}

inline Vector3 operator/(const Vector3& V1, const Vector3& V2) noexcept
{
    using namespace rtm;
    return vector_div(static_cast<Vector3::SIMDValueType>(V1), static_cast<Vector3::SIMDValueType>(V2));
}

inline Vector3 operator/(const Vector3& V, float S) noexcept
{
    using namespace rtm;
    return vector_div(static_cast<Vector3::SIMDValueType>(V), vector_set(S));
}

inline Vector3 operator*(float S, const Vector3& V) noexcept
{
    using namespace rtm;
    return vector_mul(static_cast<Vector3::SIMDValueType>(V), S);
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool Vector3::InBounds(const Vector3& Bounds) const noexcept
{
    using namespace rtm;
    return vector_all_less_equal(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(Bounds)) && vector_all_greater_equal(static_cast<SIMDValueType>(*this), vector_neg(static_cast<SIMDValueType>(Bounds)));
}

inline float Vector3::Length() const noexcept
{
    using namespace rtm;
    return vector_length(static_cast<SIMDValueType>(*this));
}

inline float Vector3::LengthSquared() const noexcept
{
    using namespace rtm;
    return vector_length_squared(static_cast<SIMDValueType>(*this));
}

inline float Vector3::Dot(const Vector3& V) const noexcept
{
    using namespace rtm;
    return vector_dot(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

inline void Vector3::Cross(const Vector3& V, Vector3& result) const noexcept
{
    using namespace rtm;
    vector_store(vector_cross3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V)), reinterpret_cast<float*>(&result));
}

inline Vector3 Vector3::Cross(const Vector3& V) const noexcept
{
    using namespace rtm;
    return vector_cross3(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(V));
}

inline void Vector3::Normalize() noexcept
{
    using namespace rtm;
    vector_store(vector_normalize3(static_cast<SIMDValueType>(*this)), reinterpret_cast<float*>(this));
}

inline void Vector3::Normalize(Vector3& result) const noexcept
{
    using namespace rtm;
    vector_store(vector_normalize3(static_cast<SIMDValueType>(*this)), reinterpret_cast<float*>(&result));
}

inline void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax) noexcept
{
    using namespace rtm;
    vector_store(vector_clamp(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(vmin), static_cast<SIMDValueType>(vmax)), reinterpret_cast<float*>(this));
}

inline void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const noexcept
{
    using namespace rtm;
    vector_store(vector_clamp(static_cast<SIMDValueType>(*this), static_cast<SIMDValueType>(vmin), static_cast<SIMDValueType>(vmax)), reinterpret_cast<float*>(&result));
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float Vector3::Distance(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_length(vector_sub(static_cast<SIMDValueType>(v2), static_cast<SIMDValueType>(v1)));
}

inline float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_length_squared(vector_sub(static_cast<SIMDValueType>(v2), static_cast<SIMDValueType>(v1)));
}

inline void Vector3::Min(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept
{
    using namespace rtm;
    vector_store(vector_min(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2)), reinterpret_cast<float*>(&result));
}

inline Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_min(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2));
}

inline void Vector3::Max(const Vector3& v1, const Vector3& v2, Vector3& result) noexcept
{
    using namespace rtm;
    vector_store(vector_max(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2)), reinterpret_cast<float*>(&result));
}

inline Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2) noexcept
{
    using namespace rtm;
    return vector_max(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2));
}

inline void Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result) noexcept
{
    using namespace rtm;
    vector_store(vector_lerp(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2), t), reinterpret_cast<float*>(&result));
}

inline Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t) noexcept
{
    using namespace rtm;
    return vector_lerp(vector_load(reinterpret_cast<const float*>(&v1)), vector_load(reinterpret_cast<const float*>(&v2)), t);
}

inline void Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result) noexcept
{
    using namespace rtm;
    t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t); // Clamp value to 0 to 1
    t = t * t * (3.f - 2.f * t);
    vector_store(vector_lerp(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2), t), reinterpret_cast<float*>(&result));
}

inline Vector3 Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t) noexcept
{
    using namespace rtm;
    t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t); // Clamp value to 0 to 1
    t = t * t * (3.f - 2.f * t);
    return vector_lerp(static_cast<SIMDValueType>(v1), static_cast<SIMDValueType>(v2), t);
}

// inline void Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline Vector3 Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline void Vector3::Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline Vector3 Vector3::Reflect(const Vector3& ivec, const Vector3& nvec) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline void Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result) noexcept
//{
//     throw std::exception("Not implemented");
// }
//
// inline Vector3 Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex) noexcept
//{
//     throw std::exception("Not implemented");
// }