#include "Viewport.h"

namespace gore
{

//------------------------------------------------------------------------------
// Comparison operators
//------------------------------------------------------------------------------

#if (__cplusplus < 202002L)
inline bool Viewport::operator==(const Viewport& vp) const noexcept
{
    return (x == vp.x && y == vp.y
            && width == vp.width && height == vp.height
            && minDepth == vp.minDepth && maxDepth == vp.maxDepth);
}

inline bool Viewport::operator!=(const Viewport& vp) const noexcept
{
    return (x != vp.x || y != vp.y
            || width != vp.width || height != vp.height
            || minDepth != vp.minDepth || maxDepth != vp.maxDepth);
}
#endif

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Viewport& Viewport::operator=(const Rect& rct) noexcept
{
    x        = float(rct.left);
    y        = float(rct.top);
    width    = float(rct.right - rct.left);
    height   = float(rct.bottom - rct.top);
    minDepth = 0.f;
    maxDepth = 1.f;
    return *this;
}

#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
inline Viewport& Viewport::operator=(const D3D11_VIEWPORT& vp) noexcept
{
    x        = vp.TopLeftX;
    y        = vp.TopLeftY;
    width    = vp.Width;
    height   = vp.Height;
    minDepth = vp.MinDepth;
    maxDepth = vp.MaxDepth;
    return *this;
}
#endif

#if defined(__d3d12_h__) || defined(__d3d12_x_h__) || defined(__XBOX_D3D12_X__)
inline Viewport& Viewport::operator=(const D3D12_VIEWPORT& vp) noexcept
{
    x        = vp.TopLeftX;
    y        = vp.TopLeftY;
    width    = vp.Width;
    height   = vp.Height;
    minDepth = vp.MinDepth;
    maxDepth = vp.MaxDepth;
    return *this;
}
#endif

//------------------------------------------------------------------------------
// Viewport operations
//------------------------------------------------------------------------------

inline float Viewport::AspectRatio() const noexcept
{
    if (width == 0.f || height == 0.f)
        return 0.f;

    return (width / height);
}


#if defined(__d3d11_h__) || defined(__d3d11_x_h__)
static_assert(sizeof(DirectX::SimpleMath::Viewport) == sizeof(D3D11_VIEWPORT), "Size mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, x) == offsetof(D3D11_VIEWPORT, TopLeftX), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, y) == offsetof(D3D11_VIEWPORT, TopLeftY), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, width) == offsetof(D3D11_VIEWPORT, Width), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, height) == offsetof(D3D11_VIEWPORT, Height), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, minDepth) == offsetof(D3D11_VIEWPORT, MinDepth), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, maxDepth) == offsetof(D3D11_VIEWPORT, MaxDepth), "Layout mismatch");
#endif

#if defined(__d3d12_h__) || defined(__d3d12_x_h__) || defined(__XBOX_D3D12_X__)
static_assert(sizeof(DirectX::SimpleMath::Viewport) == sizeof(D3D12_VIEWPORT), "Size mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, width) == offsetof(D3D12_VIEWPORT, Width), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, height) == offsetof(D3D12_VIEWPORT, Height), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "Layout mismatch");
static_assert(offsetof(DirectX::SimpleMath::Viewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "Layout mismatch");
#endif

Rect Viewport::ComputeTitleSafeArea(unsigned int backBufferWidth, unsigned int backBufferHeight) noexcept
{
    const float safew = (float(backBufferWidth) + 19.f) / 20.f;
    const float safeh = (float(backBufferHeight) + 19.f) / 20.f;

    Rect rct;
    rct.left = static_cast<int32_t>(safew);
    rct.top = static_cast<int32_t>(safeh);
    rct.right = static_cast<int32_t>(float(backBufferWidth) - safew + 0.5f);
    rct.bottom = static_cast<int32_t>(float(backBufferHeight) - safeh + 0.5f);

    return rct;
}

} // namespace gore