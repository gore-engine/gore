#include "RenderContext.h"

namespace gore
{
RenderContext::RenderContext(vk::raii::Device* device) :
    m_device(device),
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

ShaderModuleHandle RenderContext::createShaderModule(const ShaderModuleDesc& desc)
{
    vk::raii::ShaderModule sm = m_device->createShaderModule(vk::ShaderModuleCreateInfo(
        vk::ShaderModuleCreateFlags(),
        desc.byteSize,
        reinterpret_cast<const uint32_t*>(desc.byteCode)));

    // FIXME: remove this copy
    ShaderModuleDesc copyDesc = desc;

    return m_ShaderModulePool.create(std::move(copyDesc), std::move(ShaderModule(std::move(sm))));
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