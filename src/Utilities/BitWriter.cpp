#include "BitWriter.h"

#include <stdexcept>

namespace gore
{
BitWriter::BitWriter(size_t maxBitsCount, bool allowResize) :
    m_AllowResize(allowResize),
    m_Index(0)
{
    m_Data.resize(maxBitsCount / 8);
}

void BitWriter::WriteUInt8(uint8_t value)
{
    if (m_Index >= m_Data.size())
    {
        if (!m_AllowResize)
        {
            std::out_of_range("BitWriter Out of space");
            return;
        }

        m_Data.resize(m_Data.size() * 2);
    }

    m_Data[m_Index++] = value;
}

void BitWriter::WriteUInt16(uint16_t value)
{
    WriteUInt8(value & 0xFF);
    WriteUInt8((value >> 8) & 0xFF);
}

void BitWriter::WriteUInt32(uint32_t value)
{
    WriteUInt16(value & 0xFFFF);
    WriteUInt16((value >> 16) & 0xFFFF);
}

} // namespace gore