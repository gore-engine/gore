#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"

namespace gore::renderer
{
class Pass;

using namespace gore::gfx;

ENGINE_CLASS(Material) final
{
public:
    NON_COPYABLE(Material)

    explicit Material() noexcept;
    ~Material() = default;

    void AddPass(const Pass& pass);

    [[nodiscard]] const std::vector<Pass>& GetPasses() const;

private:
    std::vector<Pass> m_Passes;
};
} // namespace gore::renderer