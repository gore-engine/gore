#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"

namespace gore::renderer
{
using namespace gore::gfx;

// FIXME: This is a temporary solution to use fixed pass names
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

ENGINE_CLASS(Material) final
{
public:
    NON_COPYABLE(Material)

    explicit Material() noexcept;
    ~Material() = default;

    void AddPass(const Pass& pass);

    [[nodiscard]] const std::vector<Pass>& GetPasses() const
    {
        return m_Passes;
    }

private:
    std::vector<Pass> m_Passes;
};
} // namespace gore::renderer