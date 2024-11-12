#pragma once
#include <span>

namespace gore
{
struct BitReader
{
public:
    BitReader(void* data, size_t count);

    uint8_t ReadUInt8();
    uint16_t ReadUInt16();
    uint32_t ReadUInt32();

    size_t GetBitsCount() const { return m_Count * 8; }
    size_t GetBitsRemaining() const { return (m_Count - m_Index) * 8; }
    size_t GetBitsRead() const { return m_Index * 8; }

private:
    std::span<uint8_t> m_Data;
    size_t m_Count;
    size_t m_Index;
};
} // namespace gore 