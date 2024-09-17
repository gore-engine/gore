#include "Prefix.h"

#include <cstddef>
#include <vector>

namespace gore::utils
{
// ArrayAllocator is a simple allocator that allocates and frees indices in an array.
class ArrayAllocator
{
    static constexpr uint32_t k_InvalidIndex = 0xffffffff;
    static constexpr uint32_t k_IncreaseSize = 1024;

public:
    ArrayAllocator(uint32_t size = 1024) noexcept;
    ~ArrayAllocator() = default;

    [[nodiscard]] uint32_t Allocate();
    void Free(uint32_t index);

    [[nodiscard]] uint32_t GetSize() const;

private:
    void IncreaseSize();

    std::vector<uint32_t> m_FreeList;
    uint32_t m_NextIndex;
    uint32_t m_Size;
};
} // namespace gore::utils