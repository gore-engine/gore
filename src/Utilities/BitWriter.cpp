#include "BitWriter.h"

#include <stdexcept>

namespace gore
{
BitWriter::BitWriter(void* data, size_t size) :
    m_AllowResize(false),
    m_Index(0)
{
    m_Data.assign(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size);
}

BitWriter::BitWriter(size_t maxByteCount, bool allowResize) :
    m_AllowResize(allowResize),
    m_Index(0)
{
    m_Data.resize(maxByteCount);
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