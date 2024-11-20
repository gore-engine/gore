#pragma once
#include <vector>
#include <cstring>
#include <stdexcept>

namespace gore
{
struct BitWriter
{
public:
    BitWriter(size_t maxBitsCount, bool allowResize = false);
    BitWriter(void* data, size_t size);

    void Flush() { m_Index = 0; }
    void ShrinkToFit() { m_Data.resize(m_Index); }

    void WriteUInt8(uint8_t value);
    void WriteUInt16(uint16_t value);
    void WriteUInt32(uint32_t value);

    template <typename T>
    void Write(const T& value)
    {
        size_t size = sizeof(T);
        if (m_Index + size > m_Data.size())
        {
            if (m_AllowResize)
            {
                m_Data.resize(m_Index + size);
            }
            else
            {
                throw std::runtime_error("BitWriter overflow");
            }
        }
        std::memcpy(&m_Data[m_Index], &value, size);
        m_Index += size;
    }
    
    bool IsAllowResize() const { return m_AllowResize; }
    
    size_t GetBitsCount() const { return m_Data.size() * 8; }
    size_t GetBitsWritten() const { return m_Index * 8; }
    size_t GetBitsRemaining() const { return GetBitsCount() - GetBitsWritten(); }

private:
    bool m_AllowResize;

    std::vector<uint8_t> m_Data;
    size_t m_Index;
};
} // namespace gore