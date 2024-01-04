#pragma once

#include <ostream>

#include "Prefix.h"
#include "Export.h"
#include "Utilities//Defines.h"
#include "Math/Defines.h"

#include "rtm/vector4f.h"

namespace gore
{

ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Vector4);
ENGINE_STRUCT(Quaternion);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Plane)
{
public:
    float x;
    float y;
    float z;
    float w;

    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream& os, const Plane& p) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::vector4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Plane);

    SHALLOW_COPYABLE(Plane);

    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(Plane);

    Plane() noexcept = default;
    constexpr Plane(float ix, float iy, float iz, float iw) noexcept :
        x(ix),
        y(iy),
        z(iz),
        w(iw)
    {
    }
    Plane(const Vector3& normal, float d) noexcept;
    Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3) noexcept;
    Plane(const Vector3& point, const Vector3& normal) noexcept;
    explicit Plane(const Vector4& v) noexcept;
    explicit Plane(const float* pArray) noexcept;

    // Properties
    Vector3 Normal() const noexcept;
    void Normal(const Vector3& normal) noexcept;

    float D() const noexcept
    {
        return w;
    }
    void D(float d) noexcept
    {
        w = d;
    }

    // Plane operations
    void Normalize() noexcept;
    void Normalize(Plane & result) const noexcept;

    float Dot(const Vector4& v) const noexcept;
    float DotCoordinate(const Vector3& position) const noexcept;
    float DotNormal(const Vector3& normal) const noexcept;

    // Static functions
    static void Transform(const Plane& plane, const Matrix4x4& M, Plane& result) noexcept;
    static Plane Transform(const Plane& plane, const Matrix4x4& M) noexcept;

    static void Transform(const Plane& plane, const Quaternion& rotation, Plane& result) noexcept;
    static Plane Transform(const Plane& plane, const Quaternion& rotation) noexcept;
    // Input quaternion must be the inverse transpose of the transformation
};

#include "Math/Plane.inl"

} // namespace gore