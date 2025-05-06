#include "Draw.h"


#include "Object/GameObject.h"

#include "Rendering/Components/Material.h"
#include "Rendering/Components/MeshRenderer.h"

namespace gore::renderer
{
bool MatchDrawFilter(const Pass& pass, const DrawCreateInfo& info)
{
    return pass.name == info.passName;
}

void PrepareDrawDataAndSort(DrawCreateInfo& info
    , std::vector<GameObject*>& gameObjects
    , std::vector<Draw>& sortedDrawData
    , Material* overrideMaterial)
{
    auto& renderContext = *RenderContext::GetInstance();

    for (uint32_t i = 0; i < gameObjects.size(); i++)
    {
        MeshRenderer* renderer = gameObjects[i]->GetComponent<MeshRenderer>();
        if (renderer == nullptr)
            continue;

        if (renderer->IsValid() == false)
            continue;

        auto handle         = overrideMaterial? overrideMaterial->GetDynamicBuffer() : renderer->GetDynamicBuffer();

        Material& material = overrideMaterial ? *overrideMaterial : renderer->GetMaterial();
        for (const auto& pass : material.GetPasses())
        {
            if (MatchDrawFilter(pass, info) == false)
                continue;

            Draw draw;
            // assert(pass.shader.empty() == false);

            draw.shader       = pass.shader;
            draw.bindGroup[0] = pass.bindGroup[0];
            draw.bindGroup[1] = pass.bindGroup[1];
            draw.bindGroup[2] = pass.bindGroup[2];

            draw.dynamicBuffer       = handle;
            draw.dynamicBufferOffset = renderer->GetDynamicBufferOffset();

            draw.vertexBuffer = renderer->GetVertexBuffer();
            draw.vertexCount  = renderer->GetVertexCount();
            draw.vertexOffset = renderer->GetVertexOffset();

            draw.indexBuffer = renderer->GetIndexBuffer();
            draw.indexCount  = renderer->GetIndexCount();
            draw.indexOffset = renderer->GetIndexOffset();

            // TODO: instance Batch
            draw.instanceCount = 1;

            sortedDrawData.push_back(draw);
        }
    }

    std::sort(sortedDrawData.begin(), sortedDrawData.end(), DrawSorter());
}

void ScheduleDraws(RenderContext& renderContext, const std::unordered_map<DrawKey, std::vector<Draw>>& drawData, const DrawKey& key, vk::CommandBuffer commandBuffer)
{
    if (drawData.find(key) == drawData.end())
        return;
    
    auto& drawList = drawData.at(key);

    for(auto& draw : drawList)
    {
        auto& graphicsPipeline = renderContext.GetGraphicsPipeline(draw.shader);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.pipeline);

        if (draw.vertexBuffer.empty() == false)
        {
            auto& vertexBuffer = renderContext.GetBuffer(draw.vertexBuffer);
            commandBuffer.bindVertexBuffers(0, {vertexBuffer.vkBuffer}, {draw.vertexOffset});
        }

        if (draw.indexBuffer.empty() == false)
        {
            auto& indexBuffer = renderContext.GetBuffer(draw.indexBuffer);
            commandBuffer.bindIndexBuffer(indexBuffer.vkBuffer, draw.indexOffset, vk::IndexType::eUint16);
        }

        if (draw.bindGroup[0].empty() == false)
        {
            auto& bindGroup = renderContext.GetBindGroup(draw.bindGroup[0]);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 0, {bindGroup.set}, {});
        }

        if (draw.bindGroup[1].empty() == false)
        {
            auto& bindGroup = renderContext.GetBindGroup(draw.bindGroup[1]);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 1, {bindGroup.set}, {});
        }

        if (draw.bindGroup[2].empty() == false)
        {
            auto& bindGroup = renderContext.GetBindGroup(draw.bindGroup[2]);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 2, {bindGroup.set}, {});
        }

        if (draw.dynamicBuffer.empty() == false)
        {
            auto& dynamicBuffer = renderContext.GetDynamicBuffer(draw.dynamicBuffer);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 3, {dynamicBuffer.set}, {draw.dynamicBufferOffset});
        }

        if (draw.indexBuffer.empty() == false)
        {
            commandBuffer.drawIndexed(draw.indexCount, draw.instanceCount, draw.indexOffset, draw.vertexOffset, draw.instanceOffset);
        }
        else
        {
            commandBuffer.draw(draw.vertexCount, draw.instanceCount, draw.vertexOffset, draw.instanceOffset);
        }
    }
}
} // namespace gore::renderer