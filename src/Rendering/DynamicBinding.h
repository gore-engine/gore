#pragma once

#include "Prefix.h"
#include "GraphicsResourcePrefix.h"

#include <vector>

namespace gore
{

class DynamicBinding
{
public:
    DynamicBinding(ShaderStage stage, uint32_t size, uint8_t* data, uint32_t offset = 0);

    void setData(const void* data, uint32_t size, uint32_t offset = 0);

    uint32_t getSize() const { return m_Size; }
    uint32_t getOffset() const { return m_Offset; }

    ShaderStage getStage() const { return m_Stage; }
    uint8_t* getData() { return m_Data.data(); }

private:
    std::vector<uint8_t> m_Data;
    ShaderStage m_Stage;
    uint32_t m_Size;
    uint32_t m_Offset;
};

} // namespace gore