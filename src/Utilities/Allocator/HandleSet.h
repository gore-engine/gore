#pragma once

#include <stack>

namespace gore::utils
{
template <typename T>
class IntHandle
{
public:
    explicit IntHandle(uint32_t value) noexcept :
        m_Value(value)
    {
    }

    [[nodiscard]] uint32_t GetValue() const noexcept
    {
        return m_Value;
    }

    static IntHandle<T> InValid() noexcept
    {
        return IntHandle<T>(0xFFFFFFFF);
    }

    [[nodiscard]] bool IsValid() const noexcept
    {
        return m_Value != 0xFFFFFFFF;
    }

    [[nodiscard]] bool operator==(const IntHandle<T>& other) const noexcept
    {
        return m_Value == other.m_Value;
    }

    [[nodiscard]] bool operator!=(const IntHandle<T>& other) const noexcept
    {
        return m_Value != other.m_Value;
    }

private:
    uint32_t m_Value;
};

template <typename T>
class HandleSet
{
public:
    static constexpr uint32_t k_MaxHandle = 0xFFFFFFFE; // Reserve 0xFFFFFFFF for invalid

    HandleSet() noexcept :
        m_NextHandle(0)
    {
    }
    ~HandleSet() = default;

    [[nodiscard]] IntHandle<T> AllocateHandle()
    {
        if (!m_FreeHandles.empty())
        {
            IntHandle<T> handle = m_FreeHandles.top();
            m_FreeHandles.pop();
            return handle;
        }
#ifdef ENGINE_DEBUG
        // Check for overflow before incrementing
        if (m_NextHandle > k_MaxHandle)
        {
            return IntHandle<T>::InValid();
        }
#endif
        return IntHandle<T>(m_NextHandle++);
    }

    void FreeHandle(IntHandle<T> handle)
    {
        // Don't allow freeing invalid handles
        if (!handle.IsValid())
        {
            return;
        }

        // Additional safety: could check for double-free by maintaining allocated set
        m_FreeHandles.push(handle);
    }

    void Clear()
    {
        m_FreeHandles = std::stack<IntHandle<T>>();
        m_NextHandle  = 0;
    }

    [[nodiscard]] uint32_t AllocatedSize() const
    {
        return m_NextHandle - static_cast<uint32_t>(m_FreeHandles.size());
    }

    [[nodiscard]] bool CanAllocate() const
    {
        return !m_FreeHandles.empty() || m_NextHandle <= k_MaxHandle;
    }

private:
    uint32_t m_NextHandle;
    std::stack<IntHandle<T>> m_FreeHandles;
};
} // namespace gore::utils