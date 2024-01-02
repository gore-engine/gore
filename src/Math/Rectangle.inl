#include "Rectangle.h"

#include "Rect.h"
#include "Vector2.h"

namespace gore
{

inline Rectangle::Rectangle(const gore::Rect& rct) noexcept :
    x(rct.left),
    y(rct.top),
    width(rct.right - rct.left),
    height(rct.bottom - rct.top)
{
}

inline Rectangle::operator Rect() noexcept
{
    Rect rct;
    rct.left   = x;
    rct.top    = y;
    rct.right  = (x + width);
    rct.bottom = (y + height);
    return rct;
}

inline bool Rectangle::operator==(const Rectangle& r) const noexcept
{
    return (x == r.x) && (y == r.y) && (width == r.width) && (height == r.height);
}

inline bool Rectangle::operator!=(const Rectangle& r) const noexcept
{
    return (x != r.x) || (y != r.y) || (width != r.width) || (height != r.height);
}

inline bool Rectangle::operator==(const Rect& rct) const noexcept
{
    return (x == rct.left) && (y == rct.top) && (width == (rct.right - rct.left)) && (height == (rct.bottom - rct.top));
}

inline bool Rectangle::operator!=(const Rect& rct) const noexcept
{
    return (x != rct.left) || (y != rct.top) || (width != (rct.right - rct.left)) || (height != (rct.bottom - rct.top));
}

inline Rectangle& Rectangle::operator=(const Rect& rct) noexcept
{
    x      = rct.left;
    y      = rct.top;
    width  = (rct.right - rct.left);
    height = (rct.bottom - rct.top);
    return *this;
}

inline bool Rectangle::IsEmpty() const noexcept
{
    return (width == 0 && height == 0 && x == 0 && y == 0);
}

inline bool Rectangle::Contains(int32_t ix, int32_t iy) const noexcept
{
    return (x <= ix) && (ix < (x + width)) && (y <= iy) && (iy < (y + height));
}

inline bool Rectangle::Contains(const Rectangle& r) const noexcept
{
    return (x <= r.x) && ((r.x + r.width) <= (x + width)) && (y <= r.y) && ((r.y + r.height) <= (y + height));
}

inline bool Rectangle::Contains(const Rect& rct) const noexcept
{
    return (x <= rct.left) && (rct.right <= (x + width)) && (y <= rct.top) && (rct.bottom <= (y + height));
}

inline bool Rectangle::Intersects(const Rectangle& r) const noexcept
{
    return (r.x < (x + width)) && (x < (r.x + r.width)) && (r.y < (y + height)) && (y < (r.y + r.height));
}

inline bool Rectangle::Intersects(const Rect& rct) const noexcept
{
    return (rct.left < (x + width)) && (x < rct.right) && (rct.top < (y + height)) && (y < rct.bottom);
}

inline void Rectangle::Offset(int32_t ox, int32_t oy) noexcept
{
    x += ox;
    y += oy;
}

//------------------------------------------------------------------------------
// Rectangle operations
//------------------------------------------------------------------------------
inline Vector2 Rectangle::Location() const noexcept
{
    return Vector2(float(x), float(y));
}

inline Vector2 Rectangle::Center() const noexcept
{
    return Vector2(float(x) + (float(width) / 2.f), float(y) + (float(height) / 2.f));
}

inline bool Rectangle::Contains(const Vector2& point) const noexcept
{
    return (float(x) <= point.x) && (point.x < float(x + width)) && (float(y) <= point.y) && (point.y < float(y + height));
}

inline void Rectangle::Inflate(int32_t horizAmount, int32_t vertAmount) noexcept
{
    x -= horizAmount;
    y -= vertAmount;
    width += horizAmount;
    height += vertAmount;
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline Rectangle Rectangle::Intersect(const Rectangle& ra, const Rectangle& rb) noexcept
{
    const int32_t righta = ra.x + ra.width;
    const int32_t rightb = rb.x + rb.width;

    const int32_t bottoma = ra.y + ra.height;
    const int32_t bottomb = rb.y + rb.height;

    const int32_t maxX = ra.x > rb.x ? ra.x : rb.x;
    const int32_t maxY = ra.y > rb.y ? ra.y : rb.y;

    const int32_t minRight  = righta < rightb ? righta : rightb;
    const int32_t minBottom = bottoma < bottomb ? bottoma : bottomb;

    Rectangle result;

    if ((minRight > maxX) && (minBottom > maxY))
    {
        result.x      = maxX;
        result.y      = maxY;
        result.width  = minRight - maxX;
        result.height = minBottom - maxY;
    }
    else
    {
        result.x      = 0;
        result.y      = 0;
        result.width  = 0;
        result.height = 0;
    }

    return result;
}

inline Rect Rectangle::Intersect(const Rect& rcta, const Rect& rctb) noexcept
{
    const int32_t maxX = rcta.left > rctb.left ? rcta.left : rctb.left;
    const int32_t maxY = rcta.top > rctb.top ? rcta.top : rctb.top;

    const int32_t minRight  = rcta.right < rctb.right ? rcta.right : rctb.right;
    const int32_t minBottom = rcta.bottom < rctb.bottom ? rcta.bottom : rctb.bottom;

    Rect result;

    if ((minRight > maxX) && (minBottom > maxY))
    {
        result.left   = maxX;
        result.top    = maxY;
        result.right  = minRight;
        result.bottom = minBottom;
    }
    else
    {
        result.left   = 0;
        result.top    = 0;
        result.right  = 0;
        result.bottom = 0;
    }

    return result;
}

inline Rectangle Rectangle::Union(const Rectangle& ra, const Rectangle& rb) noexcept
{
    const int32_t righta = ra.x + ra.width;
    const int32_t rightb = rb.x + rb.width;

    const int32_t bottoma = ra.y + ra.height;
    const int32_t bottomb = rb.y + rb.height;

    const int minX = ra.x < rb.x ? ra.x : rb.x;
    const int minY = ra.y < rb.y ? ra.y : rb.y;

    const int maxRight  = righta > rightb ? righta : rightb;
    const int maxBottom = bottoma > bottomb ? bottoma : bottomb;

    Rectangle result;
    result.x      = minX;
    result.y      = minY;
    result.width  = maxRight - minX;
    result.height = maxBottom - minY;
    return result;
}

inline Rect Rectangle::Union(const Rect& rcta, const Rect& rctb) noexcept
{
    Rect result;
    result.left   = rcta.left < rctb.left ? rcta.left : rctb.left;
    result.top    = rcta.top < rctb.top ? rcta.top : rctb.top;
    result.right  = rcta.right > rctb.right ? rcta.right : rctb.right;
    result.bottom = rcta.bottom > rctb.bottom ? rcta.bottom : rctb.bottom;
    return result;
}


} // namespace gore