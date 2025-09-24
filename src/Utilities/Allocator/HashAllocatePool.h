#pragma once

#include "Prefix.h"

#include "ArrayAllocator.h"

#include <unordered_map>

namespace gore::utils
{
template <typename T>
class HashAllocatePool
{
public:
    HashAllocatePool(uint32_t size = 1024) noexcept :
        m_Allocator(size)
    {
    }
    ~HashAllocatePool() = default;

    [[nodiscard]] bool AddInstance(const T& obj, uint32_t& outIndex)
    {
        if (m_HashToIndexMap.find(obj) != m_HashToIndexMap.end())
        {
            outIndex = m_HashToIndexMap[obj];
            return false; // Already exists
        }

        uint32_t index = m_Allocator.Allocate();
        if (index == ArrayAllocator::k_InvalidIndex)
        {
            return false; // Allocation failed
        }

        m_HashToIndexMap[obj] = index;
        outIndex              = index;
        return true; // New allocation
    }

    void RemoveInstance(const T& obj)
    {
        auto it = m_HashToIndexMap.find(obj);
        if (it != m_HashToIndexMap.end())
        {
            m_Allocator.Free(it->second);
            m_HashToIndexMap.erase(it);
        }
    }

    T& GetInstance(uint32_t index)
    {
        for (const auto& pair : m_HashToIndexMap)
        {
            if (pair.second == index)
            {
                return const_cast<T&>(pair.first);
            }
        }
        throw std::out_of_range("Index not found");
    }

    [[nodiscard]] uint32_t GetSize() const
    {
        return m_Allocator.GetSize();
    }

    [[nodiscard]] bool IsFull() const
    {
        return m_Allocator.IsFull();
    }

private:
    ArrayAllocator m_Allocator;
    std::unordered_map<T, uint32_t> m_HashToIndexMap;
};
} // namespace gore::utils