#include "Draw.h"

#include "Rendering/Components/MeshRenderer.h"

namespace gore::renderer
{
void PrepareDrawDataAndSort(std::vector<MeshRenderer>& renderers, std::vector<Draw>& sortedDrawData)
{
    for (auto& renderer : renderers)
    {
        if (!renderer.HasVertexData() || !renderer.HasIndexData())
            continue;

        Draw draw;
        draw.shader       = renderer.GetMaterial()->GetGraphicsPipeline();
        draw.bindGroup[0] = renderer.GetMaterial()->GetBindGroup();
        draw.vertexBuffer = renderer.GetVertexBuffer();
        draw.indexBuffer  = renderer.GetIndexBuffer();
        draw.indexCount   = renderer.GetIndexCount();
        draw.vertexCount  = renderer.GetVertexCount();

        sortedDrawData.push_back(draw);
    }

    std::sort(sortedDrawData.begin(), sortedDrawData.end(), DrawSorter());
}
} // namespace gore::renderer