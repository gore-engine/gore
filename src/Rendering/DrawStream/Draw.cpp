#include "Draw.h"

#include "Rendering/RenderContext.h"

#include "Object/GameObject.h"

#include "Rendering/Components/Material.h"
#include "Rendering/Components/MeshRenderer.h"

namespace gore::renderer
{
bool MatchDrawFilter(const Pass& pass, const DrawCreateInfo& info)
{
    return pass.name == info.passName;
}

void PrepareDrawDataAndSort(DrawCreateInfo& info, std::vector<GameObject*>& gameObjects, std::vector<Draw>& sortedDrawData)
{
    auto& renderContext = *RenderContext::GetInstance();

    for (uint32_t i = 0; i < gameObjects.size(); i++)
    {
        MeshRenderer* renderer = gameObjects[i]->GetComponent<MeshRenderer>();
        if (renderer == nullptr)
            continue;

        if (renderer->IsValid() == false)
            continue;

        auto handle         = renderer->GetDynamicBuffer();
        auto& dynamicBuffer = renderContext.GetDynamicBuffer(handle);

        Material& material = renderer->GetMaterial();
        for (const auto& pass : material.GetPasses())
        {
            Draw draw;
            assert(pass.shader.empty() == false);

            draw.shader       = pass.shader;
            draw.bindGroup[0] = pass.bindGroup[0];
            draw.bindGroup[1] = pass.bindGroup[1];
            draw.bindGroup[2] = pass.bindGroup[2];

            draw.dynamicBuffer       = handle;
            draw.dynamicBufferOffset = dynamicBuffer.offset;

            draw.vertexBuffer = renderer->GetVertexBuffer();
            draw.vertexCount  = renderer->GetVertexCount();
            draw.vertexOffset = renderer->GetVertexOffset();

            draw.indexBuffer = renderer->GetIndexBuffer();
            draw.indexCount  = renderer->GetIndexCount();
            draw.indexOffset = renderer->GetIndexOffset();

            sortedDrawData.push_back(draw);
        }
    }

    std::sort(sortedDrawData.begin(), sortedDrawData.end(), DrawSorter());
}
} // namespace gore::renderer