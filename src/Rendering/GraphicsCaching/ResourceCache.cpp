#include "Rendering/GraphicsCaching/ResourceCache.h"

namespace gore::gfx
{
void ClearCache(ResourceCache& cache, vk::Device device)
{
    {
        for (auto& [hash, bindLayout] : cache.bindLayouts)
        {
            device.destroyDescriptorSetLayout(bindLayout.layout);
        }
     
        for (auto& [hash, pipelineLayout] : cache.pipelineLayouts)
        {
            device.destroyPipelineLayout(pipelineLayout.layout);
        }
        cache.bindLayouts.clear();
    }
}
} // namespace gore::gfx