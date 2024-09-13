#include "Prefix.h"

#include <cstddef>
#include <vector>

namespace gore::utils
{
class ArrayAllocator
{
    static constexpr uint32_t k_InvalidIndex = 0xffffffff;
    static constexpr uint32_t k_IncreaseSize = 1024;
};
} // namespace gore::utils