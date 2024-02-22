#include "Core/Log.h"
#include "DynamicBinding.h"

#include <cstring>

namespace gore
{
DynamicBinding::DynamicBinding(ShaderStage stage, uint32_t size, uint8_t* data, uint32_t offset)
{
    m_Stage  = stage;
    m_Size   = size;
    m_Offset = offset;
    m_Data.resize(size);
    memcpy(m_Data.data(), data, size);
}
void DynamicBinding::setData(const void* data, uint32_t size, uint32_t offset)
{
    if (offset + size > m_Size)
    {
        LOG_STREAM(DEBUG) << "Too large data!" <<std::endl;
        size = m_Size - offset;
    }

    memcpy(m_Data.data() + offset, data, size);
}
} // namespace gore