#include "Color.h"

namespace gore
{

std::ostream& operator<<(std::ostream& os, const Color& c) noexcept
{
    return os << "Color(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")";
}

inline Color::Color(const float* pArray) noexcept :
    r(pArray[0]),
    g(pArray[1]),
    b(pArray[2]),
    a(pArray[3])
{
}

inline Color& Color::operator=(const gore::Color::SIMDValueType& F) noexcept
{
    rtm::vector_store(F, reinterpret_cast<float*>(this));
    return *this;
}

// Common Values
const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Yellow(1.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Magenta(1.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Cyan(0.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Transparent(0.0f, 0.0f, 0.0f, 0.0f);
const Color Color::Gray(0.5f, 0.5f, 0.5f, 1.0f);
const Color Color::Grey(0.5f, 0.5f, 0.5f, 1.0f);
const Color Color::Orange(1.0f, 0.5f, 0.0f, 1.0f);
const Color Color::Purple(0.5f, 0.0f, 0.5f, 1.0f);
const Color Color::Pink(1.0f, 0.75f, 0.8f, 1.0f);
const Color Color::Brown(0.6f, 0.4f, 0.2f, 1.0f);
const Color Color::Gold(1.0f, 0.84f, 0.0f, 1.0f);
const Color Color::Silver(0.75f, 0.75f, 0.75f, 1.0f);
const Color Color::Bronze(0.8f, 0.5f, 0.2f, 1.0f);
const Color Color::SkyBlue(0.53f, 0.81f, 0.92f, 1.0f);
const Color Color::LightBlue(0.68f, 0.85f, 0.9f, 1.0f);
const Color Color::LightRed(1.0f, 0.6f, 0.6f, 1.0f);
const Color Color::LightGreen(0.6f, 1.0f, 0.6f, 1.0f);
const Color Color::LightYellow(1.0f, 1.0f, 0.6f, 1.0f);
const Color Color::LightMagenta(1.0f, 0.6f, 1.0f, 1.0f);
const Color Color::LightCyan(0.6f, 1.0f, 1.0f, 1.0f);
const Color Color::LightGray(0.75f, 0.75f, 0.75f, 1.0f);
const Color Color::LightGrey(0.75f, 0.75f, 0.75f, 1.0f);
const Color Color::DarkGray(0.25f, 0.25f, 0.25f, 1.0f);
const Color Color::DarkGrey(0.25f, 0.25f, 0.25f, 1.0f);
const Color Color::DarkRed(0.5f, 0.0f, 0.0f, 1.0f);
const Color Color::DarkGreen(0.0f, 0.5f, 0.0f, 1.0f);
const Color Color::DarkBlue(0.0f, 0.0f, 0.5f, 1.0f);
const Color Color::DarkYellow(0.5f, 0.5f, 0.0f, 1.0f);
const Color Color::DarkMagenta(0.5f, 0.0f, 0.5f, 1.0f);
const Color Color::DarkCyan(0.0f, 0.5f, 0.5f, 1.0f);
const Color Color::DarkOrange(1.0f, 0.55f, 0.0f, 1.0f);
const Color Color::DarkPurple(0.5f, 0.0f, 0.5f, 1.0f);
const Color Color::DarkPink(1.0f, 0.08f, 0.58f, 1.0f);

} // namespace gore
