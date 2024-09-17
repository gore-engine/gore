#include "ArrayAllocator.h"

#include <cassert>

namespace gore::utils
{
ArrayAllocator::ArrayAllocator(uint32_t size) noexcept :
    m_NextIndex(0),
    m_Size(size)
{
    assert(size > 0);

    m_FreeList.reserve(size);

    for (uint32_t i = 0; i < size; ++i)
    {
        m_FreeList.push_back(i + 1);
    }

    m_FreeList[size - 1] = k_InvalidIndex;
}

void ArrayAllocator::IncreaseSize()
{
    m_Size += k_IncreaseSize;
    m_FreeList.reserve(m_Size);

    for (uint32_t i = m_Size - k_IncreaseSize; i < m_Size; ++i)
    {
        m_FreeList.push_back(i + 1);
    }

    m_FreeList[m_Size - 1]                  = k_InvalidIndex;
    m_NextIndex                             = m_Size - k_IncreaseSize;
    m_FreeList[m_Size - k_IncreaseSize - 1] = m_NextIndex;
}

uint32_t ArrayAllocator::Allocate()
{
    if (m_NextIndex == k_InvalidIndex)
    {
        IncreaseSize();
    }

    uint32_t index    = m_NextIndex;
    m_NextIndex       = m_FreeList[m_NextIndex];
    m_FreeList[index] = k_InvalidIndex;

    return index;
}

void ArrayAllocator::Free(uint32_t index)
{
    assert(index < m_Size);
    assert(m_FreeList[index] == k_InvalidIndex);

    m_FreeList[index] = m_NextIndex;
    m_NextIndex       = index;
}

uint32_t ArrayAllocator::GetSize() const
{
    return m_Size;
}

} // namespace gore::utils