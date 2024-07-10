#pragma once

namespace gore::utils
{
inline size_t AlignUp(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}
} // namespace gore