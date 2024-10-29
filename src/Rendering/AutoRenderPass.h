#pragma once

#include "Export.h"

#include "Rendering/RenderContext.h"

namespace gore::gfx
{
ENGINE_STRUCT(AutoRenderPass)
{
    AutoRenderPass(RenderContext * renderContext, RenderPassDesc & renderPassDesc);
    ~AutoRenderPass();

    RenderPass GetRenderPass() const
    {
        return m_RenderPass;
    }

private:
    RenderContext* m_RenderContext = nullptr;
    RenderPass m_RenderPass        = {};
};
} // namespace gore::gfx