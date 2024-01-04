#pragma once

#if (__cplusplus >= 202002L)
// default comparison operators (==, <=>)
#include <compare>
#endif

#include "Export.h"

#include "Utilities/Defines.h"
#include "Math/Defines.h"
#include "Rect.h"

namespace gore
{

ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Matrix4x4);

ENGINE_STRUCT(Viewport)
{
public:
    // TODO: Confirm if (x, y, width, height) should be integers or floats.
    // SimpleMath uses integers, but I'm not sure if that's the best choice.
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;

public:
    Viewport() noexcept :
        x(0.f),
        y(0.f),
        width(0.f),
        height(0.f),
        minDepth(0.f),
        maxDepth(1.f)
    {
    }
    constexpr Viewport(float ix, float iy, float iw, float ih, float iminz = 0.f, float imaxz = 1.f) noexcept :
        x(ix),
        y(iy),
        width(iw),
        height(ih),
        minDepth(iminz),
        maxDepth(imaxz)
    {
    }
    explicit Viewport(const Rect& rct) noexcept :
        x(float(rct.left)),
        y(float(rct.top)),
        width(float(rct.right - rct.left)),
        height(float(rct.bottom - rct.top)),
        minDepth(0.f),
        maxDepth(1.f)
    {
    }

#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
    // Direct3D 11 interop
    explicit Viewport(const D3D11_VIEWPORT& vp) noexcept :
        x(vp.TopLeftX),
        y(vp.TopLeftY),
        width(vp.Width),
        height(vp.Height),
        minDepth(vp.MinDepth),
        maxDepth(vp.MaxDepth)
    {
    }

    operator D3D11_VIEWPORT() noexcept
    {
        return *reinterpret_cast<const D3D11_VIEWPORT*>(this);
    }
    const D3D11_VIEWPORT* Get11() const noexcept
    {
        return reinterpret_cast<const D3D11_VIEWPORT*>(this);
    }
    Viewport& operator=(const D3D11_VIEWPORT& vp) noexcept;
#endif

#if defined(__d3d12_h__) || defined(__d3d12_x_h__) || defined(__XBOX_D3D12_X__)
    // Direct3D 12 interop
    explicit Viewport(const D3D12_VIEWPORT& vp) noexcept :
        x(vp.TopLeftX),
        y(vp.TopLeftY),
        width(vp.Width),
        height(vp.Height),
        minDepth(vp.MinDepth),
        maxDepth(vp.MaxDepth)
    {
    }

    operator D3D12_VIEWPORT() noexcept
    {
        return *reinterpret_cast<const D3D12_VIEWPORT*>(this);
    }
    const D3D12_VIEWPORT* Get12() const noexcept
    {
        return reinterpret_cast<const D3D12_VIEWPORT*>(this);
    }
    Viewport& operator=(const D3D12_VIEWPORT& vp) noexcept;
#endif

    SHALLOW_COPYABLE(Viewport);

    // Comparison operators
#if (__cplusplus >= 202002L)
    bool operator==(const Viewport&) const  = default;
    auto operator<=>(const Viewport&) const = default;
#else
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(Viewport);
#endif

    // Assignment operators
    Viewport& operator=(const Rect& rct) noexcept;

    // Viewport operations
    [[nodiscard]] float AspectRatio() const noexcept;

    Vector3 Project(const Vector3& p, const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& world) const noexcept;
    void Project(const Vector3& p, const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& world, Vector3& result) const noexcept;

    Vector3 Unproject(const Vector3& p, const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& world) const noexcept;
    void Unproject(const Vector3& p, const Matrix4x4& proj, const Matrix4x4& view, const Matrix4x4& world, Vector3& result) const noexcept;

    // Static methods
    static Rect DECLTYPE ComputeTitleSafeArea(unsigned int backBufferWidth, unsigned int backBufferHeight) noexcept;
};

} // namespace gore