#pragma once

#include "Prefix.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"

namespace gore::renderer
{
using namespace gore::gfx;

struct PassID
{
    const char* k_ShadowPassName = "ShadowPass";
    const char* k_ForwardPassName = "ForwardPass";
    const char* k_GBufferPassName = "GBufferPass";
};

struct Pass
{
    GraphicsPipelineHandle shader = {};
    BindGroupHandle bindGroup[3] = {};

    inline bool operator==(const Pass& other) const
    {
        return shader == other.shader && bindGroup[0] == other.bindGroup[0] && bindGroup[1] == other.bindGroup[1] && bindGroup[2] == other.bindGroup[2];
    }

    inline bool operator!=(const Pass& other) const
    {
        return !(*this == other);
    }
};
} // namespace gore::renderer