#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"

namespace gore::renderer
{
using namespace gore::gfx;

ENGINE_CLASS(Material) final
{
public:
    NON_COPYABLE(Material)

    explicit Material() noexcept;
    ~Material() = default;

    GETTER_SETTER(GraphicsPipelineHandle, GraphicsPipeline)
    GETTER_SETTER(BindGroupHandle, BindGroup)

private:
    GraphicsPipelineHandle m_GraphicsPipeline;
    BindGroupHandle m_BindGroup;
};
} // namespace gore::renderer