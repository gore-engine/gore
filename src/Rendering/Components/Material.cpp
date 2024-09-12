#include "Material.h"

#include "Rendering/RenderContext.h"

namespace gore::renderer
{
Material::Material() noexcept
{
    
}

void Material::AddPass(const Pass& pass)
{
    

    m_Passes.push_back(pass);
}

} // namespace gore::renderer
