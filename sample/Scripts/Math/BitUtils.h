#pragma once

#include "Export.h"

namespace utils
{
inline bool IsPowerOfTwo(size_t value)
{
    return (value & (value - 1)) == 0;
}

inline size_t RoundUpToPowerOfTwo(size_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

inline size_t RoundDownToPowerOfTwo(size_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value - (value >> 1);
}

inline size_t DivideAndRoundUp(size_t dividend, size_t divisor)
{
    return (dividend + divisor - 1) & ~(divisor - 1);
}
} // namespace gore::utils