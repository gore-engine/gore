#pragma once
#include <vector>

namespace gore
{
struct BitWriter
{
public:
    BitWriter(size_t maxBitsCount, bool allowResize = false);

    void WriteUInt8(uint8_t value);
    void WriteUInt16(uint16_t value);
    void WriteUInt32(uint32_t value);
    
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