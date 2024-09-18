#include "Prefix.h"

#include <cstddef>
#include <vector>

namespace gore::utils
{
// ArrayAllocator is a simple allocator that allocates and frees indices in an array.
class ArrayAllocator
{
public:
    static constexpr uint32_t k_InvalidIndex = 0xffffffff;
    static constexpr uint32_t k_IncreaseSize = 1024;

    static_assert(k_InvalidIndex == static_cast<uint32_t>(-1), "Invalid index must be 0xffffffff");
    static_assert(k_IncreaseSize > 0, "Increase size must be greater than 0");

    ArrayAllocator(uint32_t size = 1024) noexcept;
    ~ArrayAllocator() = default;

    [[nodiscard]] uint32_t Allocate();
    void Free(uint32_t index);

    [[nodiscard]] uint32_t GetSize() const;
    [[nodiscard]] inline bool IsFull() const;

private:
    void IncreaseSize();

    std::vector<uint32_t> m_FreeList;
    uint32_t m_NextIndex;
    uint32_t m_Size;
};
} // namespace gore::utils