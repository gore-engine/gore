#include "BitReader.h"

#include <stdexcept>

namespace gore
{
BitReader::BitReader(void* data, size_t count)
    : m_Data(reinterpret_cast<uint8_t*>(data), reinterpret_cast<uint8_t*>(data) + count)
    , m_Count(count)
    , m_Index(0)
{
}

uint8_t BitReader::ReadUInt8()
{
    if (m_Index + sizeof(uint8_t) > m_Count)
    {
        throw std::out_of_range("BitReader::ReadUInt8");
    }

    uint8_t value = m_Data[m_Index];
    m_Index += sizeof(uint8_t);
    return value;
}

uint16_t BitReader::ReadUInt16()
{
    if (m_Index + sizeof(uint16_t) > m_Count)
    {
        throw std::out_of_range("BitReader::ReadUInt16");
    }

    uint16_t value = *reinterpret_cast<uint16_t*>(&m_Data[m_Index]);
    m_Index += sizeof(uint16_t);
    return value;
}

uint32_t BitReader::ReadUInt32()
{
    if (m_Index + sizeof(uint32_t) > m_Count)
    {
        throw std::out_of_range("BitReader::ReadUInt32");
    }

    uint32_t value = *reinterpret_cast<uint32_t*>(&m_Data[m_Index]);
    m_Index += sizeof(uint32_t);
    return value;
}
} // namespace gore