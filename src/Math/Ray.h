#pragma once

#include "Export.h"

#include "Utilities/Defines.h"
#include "Math/Defines.h"

#include "Vector3.h"

namespace gore
{

ENGINE_STRUCT(Ray)
{
public:
    Vector3 origin;
    Vector3 direction;

public:
    Ray() noexcept :
        origin(0),
        direction(0, 0, 1)
    {
    }
    Ray(const Vector3& pos, const Vector3& dir) noexcept :
        origin(pos),
        direction(dir)
    {
    }

    SHALLOW_COPYABLE(Ray);

    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(Ray);

    // Ray operations
    //    bool Intersects(const BoundingSphere& sphere, _Out_ float& Dist) const noexcept;
    //    bool Intersects(const BoundingBox& box, _Out_ float& Dist) const noexcept;
    //    bool Intersects(const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, float& Dist) const noexcept;
    //    bool Intersects(const Plane& plane, _Out_ float& Dist) const noexcept;
};

};

} // namespace gore