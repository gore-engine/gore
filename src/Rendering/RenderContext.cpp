#include "RenderContext.h"

namespace gore
{
RenderContext::RenderContext(vk::raii::Device* device) :
    m_DevicePtr(device),
    m_ShaderModulePool()
{
}

RenderContext::~RenderContext()
{
    clear();
}

void RenderContext::clear()
{
    m_ShaderModulePool.clear();
}

ShaderModuleHandle RenderContext::createShaderModule(ShaderModuleDesc&& desc)
{
    return m_ShaderModulePool.create(
        std::move(desc),
        std::move(ShaderModule(
            m_DevicePtr->createShaderModule(
                vk::ShaderModuleCreateInfo(
                    {},
                    desc.byteSize,
                    reinterpret_cast<const uint32_t*>(desc.byteCode))))));
}

const ShaderModule& RenderContext::getShaderModule(ShaderModuleHandle handle)
{
    return m_ShaderModulePool.getObject(handle);
}

void RenderContext::destroyShaderModule(ShaderModuleHandle handle)
{
    m_ShaderModulePool.destroy(handle);
}

} // namespace gore