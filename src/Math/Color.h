#pragma once

#include <ostream>

#include "Export.h"
#include "Utilities/Defines.h"
#include "Math/Defines.h"
#include "rtm/vector4f.h"

namespace gore
{

ENGINE_STRUCT(Color)
{
public:
    float r;
    float g;
    float b;
    float a;

    friend std::ostream& operator<<(std::ostream& os, const Color& c) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::vector4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Color);

    SHALLOW_COPYABLE(Color);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Color);
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(Color);

    Color() noexcept = default;
    constexpr Color(float ir, float ig, float ib, float ia) noexcept :
        r(ir),
        g(ig),
        b(ib),
        a(ia)
    {
    }
    explicit Color(const float* pArray) noexcept;

    Color& operator=(const ValueType& F) noexcept;
    //    Color& operator= (const DirectX::PackedVector::XMCOLOR& Packed) noexcept;
    //    Color& operator= (const DirectX::PackedVector::XMUBYTEN4& Packed) noexcept;
    Color& operator+=(const Color& c) noexcept;
    Color& operator-=(const Color& c) noexcept;
    Color& operator*=(const Color& c) noexcept;
    Color& operator*=(float S) noexcept;
    Color& operator/=(const Color& c) noexcept;

    // Common Values
    static const Color Black;
    static const Color White;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Magenta;
    static const Color Cyan;
    static const Color Transparent;
    static const Color Gray;
    static const Color Grey;
    static const Color Orange;
    static const Color Purple;
    static const Color Pink;
    static const Color Brown;
    static const Color Gold;
    static const Color Silver;
    static const Color Bronze;
    static const Color SkyBlue;
    static const Color LightBlue;
    static const Color LightRed;
    static const Color LightGreen;
    static const Color LightYellow;
    static const Color LightMagenta;
    static const Color LightCyan;
    static const Color LightGray;
    static const Color LightGrey;
    static const Color DarkGray;
    static const Color DarkGrey;
    static const Color DarkRed;
    static const Color DarkGreen;
    static const Color DarkBlue;
    static const Color DarkYellow;
    static const Color DarkMagenta;
    static const Color DarkCyan;
    static const Color DarkOrange;
    static const Color DarkPurple;
    static const Color DarkPink;
};

// Binary operators
Color operator+(const Color& C1, const Color& C2) noexcept;
Color operator-(const Color& C1, const Color& C2) noexcept;
Color operator*(const Color& C1, const Color& C2) noexcept;
Color operator*(const Color& C, float S) noexcept;
Color operator/(const Color& C1, const Color& C2) noexcept;
Color operator*(float S, const Color& C) noexcept;

} // namespace gore