#include "AutoRenderPass.h"

namespace gore::gfx
{
AutoRenderPass::AutoRenderPass(RenderContext* renderContext, RenderPassDesc& renderPassDesc)
{
    m_RenderContext = renderContext;
    m_RenderPass    = renderContext->CreateRenderPass(std::move(renderPassDesc));
}

AutoRenderPass::~AutoRenderPass()
{
    m_RenderContext->DestroyRenderPass(m_RenderPass);
}

} // namespace gore::gfx