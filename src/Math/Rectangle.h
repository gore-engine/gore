#pragma once

#include "Export.h"
#include "Math/Defines.h"

namespace gore
{

struct Rect;
struct Vector2;

ENGINE_STRUCT(Rectangle)
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;

    // Creators
    Rectangle() noexcept = default;
    constexpr Rectangle(int32_t ix, int32_t iy, int32_t iw, int32_t ih) noexcept :
        x(ix),
        y(iy),
        width(iw),
        height(ih)
    {
    }
    explicit Rectangle(const Rect& rct) noexcept;

    SHALLOW_COPYABLE(Rectangle);

    operator Rect() noexcept;

#ifdef __cplusplus_winrt
    operator Windows::Foundation::Rect() noexcept
    {
        return Windows::Foundation::Rect(float(x), float(y), float(width), float(height));
    }
#endif

    // Comparison operators
#if (__cplusplus >= 202002L)
    bool operator==(const Rectangle&) const  = default;
    auto operator<=>(const Rectangle&) const = default;
#else
    bool operator==(const Rectangle& r) const noexcept;
    bool operator!=(const Rectangle& r) const noexcept;
#endif
    bool operator==(const Rect& rct) const noexcept;
    bool operator!=(const Rect& rct) const noexcept;

    // Assignment operators
    Rectangle& operator=(_In_ const Rect& rct) noexcept;

    // Rectangle operations
    [[nodiscard]] Vector2 Location() const noexcept;
    [[nodiscard]] Vector2 Center() const noexcept;

    [[nodiscard]] bool IsEmpty() const noexcept;

    [[nodiscard]] bool Contains(int32_t ix, int32_t iy) const noexcept;
    [[nodiscard]] bool Contains(const Vector2& point) const noexcept;
    [[nodiscard]] bool Contains(const Rectangle& r) const noexcept;
    [[nodiscard]] bool Contains(const Rect& rct) const noexcept;

    void Inflate(int32_t horizAmount, int32_t vertAmount) noexcept;

    [[nodiscard]] bool Intersects(const Rectangle& r) const noexcept;
    [[nodiscard]] bool Intersects(const Rect& rct) const noexcept;

    void Offset(int32_t ox, int32_t oy) noexcept;

    // Static functions
    static Rectangle Intersect(const Rectangle& ra, const Rectangle& rb) noexcept;
    static Rect Intersect(const Rect& rcta, const Rect& rctb) noexcept;

    static Rectangle Union(const Rectangle& ra, const Rectangle& rb) noexcept;
    static Rect Union(const Rect& rcta, const Rect& rctb) noexcept;
};

} // namespace gore