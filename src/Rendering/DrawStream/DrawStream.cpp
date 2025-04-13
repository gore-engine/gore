#include "DrawStream.h"

#include "Utilities/BitWriter.h"
#include "Utilities/BitReader.h"

namespace gore::renderer
{
static inline void WriteDraw(BitWriter& writer, const DrawStateMask mask, const Draw draw)
{
    if (mask.mask == 0)
    {
        return;
    }

    if (mask.shader != 0)
    {
        writer.Write(draw.shader);
    }

    if (mask.bindgroup0 != 0)
    {
        writer.Write(draw.bindGroup[0]);
    }

    if (mask.bindgroup1 != 0)
    {
        writer.Write(draw.bindGroup[1]);
    }

    if (mask.bindgroup2 != 0)
    {
        writer.Write(draw.bindGroup[2]);
    }

    if (mask.indexBuffer != 0)
    {
        writer.Write(draw.indexBuffer);
    }

    if (mask.vertexBuffer != 0)
    {
        writer.Write(draw.vertexBuffer);
    }

    if (mask.dynamicBuffer != 0)
    {
        writer.Write(draw.dynamicBuffer);
    }

    if (mask.indexOffset != 0)
    {
        writer.Write(draw.indexOffset);
    }

    if (mask.vertexOffset != 0)
    {
        writer.Write(draw.vertexOffset);
    }

    if (mask.instanceOffset != 0)
    {
        writer.Write(draw.instanceOffset);
    }

    if (mask.instanceCount != 0)
    {
        writer.Write(draw.instanceCount);
    }

    if (mask.dynamicBufferOffset != 0)
    {
        writer.Write(draw.dynamicBufferOffset);
    }

    if (mask.indexCount != 0)
    {
        writer.Write(draw.indexCount);
    }
}

void CreateDrawStreamFromDrawData(const std::vector<Draw>& drawData, DrawStream& drawStream)
{
    if (drawData.empty())
    {
        return;
    }

    // Calculate the size of the draw stream
    const int maxPerDrawByteSize = sizeof(DrawStateMask) + sizeof(Draw);
    const size_t maxSize         = maxPerDrawByteSize * drawData.size();

    // Loop through the draw data and create the draw stream
    Draw lastDraw = drawData[0];
    bool first    = true;

    BitWriter writer(maxSize);
    DrawStateMask mask = {};

    for (const auto& draw : drawData)
    {
        mask.mask = 0;

        if (first)
        {
            mask.shader              = draw.shader.empty() == false;
            mask.bindgroup0          = draw.bindGroup[0].empty() == false;
            mask.bindgroup1          = draw.bindGroup[1].empty() == false;
            mask.bindgroup2          = draw.bindGroup[2].empty() == false;
            mask.indexBuffer         = draw.indexBuffer.empty() == false;
            mask.vertexBuffer        = draw.vertexBuffer.empty() == false;
            mask.dynamicBuffer       = draw.dynamicBuffer.empty() == false;
            mask.indexOffset         = draw.indexOffset != 0;
            mask.vertexOffset        = draw.vertexOffset != 0;
            mask.instanceOffset      = draw.instanceOffset != 0;
            mask.instanceCount       = draw.instanceCount != 0;
            mask.dynamicBufferOffset = draw.dynamicBufferOffset != 0;
            mask.indexCount          = draw.indexCount != 0;

            if (mask.mask != 0)
                writer.Write(mask);

            WriteDraw(writer, mask, draw);

            lastDraw = draw;

            first = false;
            continue;
        }

        mask.shader              = lastDraw.shader != draw.shader;
        mask.bindgroup0          = lastDraw.bindGroup[0] != draw.bindGroup[0];
        mask.bindgroup1          = lastDraw.bindGroup[1] != draw.bindGroup[1];
        mask.bindgroup2          = lastDraw.bindGroup[2] != draw.bindGroup[2];
        mask.indexBuffer         = lastDraw.indexBuffer != draw.indexBuffer;
        mask.vertexBuffer        = lastDraw.vertexBuffer != draw.vertexBuffer;
        mask.dynamicBuffer       = lastDraw.dynamicBuffer != draw.dynamicBuffer;
        mask.indexOffset         = lastDraw.indexOffset != draw.indexOffset;
        mask.vertexOffset        = lastDraw.vertexOffset != draw.vertexOffset;
        mask.instanceOffset      = lastDraw.instanceOffset != draw.instanceOffset;
        mask.instanceCount       = lastDraw.instanceCount != draw.instanceCount;
        mask.dynamicBufferOffset = lastDraw.dynamicBufferOffset != draw.dynamicBufferOffset;
        mask.indexCount          = lastDraw.indexCount != draw.indexCount;
        
        writer.Write(mask);
        WriteDraw(writer, mask, draw);
    }

    writer.ShrinkToFit();

    drawStream.data.assign(writer.GetData(), writer.GetData() + writer.GetByteWritten());
}

void ScheduleDrawStream(RenderContext& renderContext, DrawStream& drawStream, vk::CommandBuffer commandBuffer, GraphicsPipelineHandle overridePipeline)
{
    BitReader reader(drawStream.data.data(), drawStream.data.size());

    DrawStateMask mask = {};

    GraphicsPipeline graphicsPipeline = {};
    uint32_t indexOffset              = 0;
    uint32_t vertexOffset             = 0;
    uint32_t instanceOffset           = 0;
    uint32_t instanceCount            = 0;
    uint32_t dynamicBufferOffset      = 0;
    uint32_t indexCount               = 0;

    while (reader.GetBitsRemaining() > 0)
    {
        mask = reader.Read<DrawStateMask>();

        if (mask.shader != 0)
        {
            auto shaderHandle = overridePipeline.empty() ? reader.Read<GraphicsPipelineHandle>() : overridePipeline;
            graphicsPipeline  = renderContext.GetGraphicsPipeline(shaderHandle);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.pipeline);
        }

        if (mask.bindgroup0 != 0)
        {
            auto bindGroupHandle = reader.Read<BindGroupHandle>();
            auto& bindGroup      = renderContext.GetBindGroup(bindGroupHandle);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 0, {bindGroup.set}, {});
        }

        if (mask.bindgroup1 != 0)
        {
            auto bindGroupHandle = reader.Read<BindGroupHandle>();
            auto& bindGroup      = renderContext.GetBindGroup(bindGroupHandle);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 1, {bindGroup.set}, {});
        }

        if (mask.bindgroup2 != 0)
        {
            auto bindGroupHandle = reader.Read<BindGroupHandle>();
            auto& bindGroup      = renderContext.GetBindGroup(bindGroupHandle);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 2, {bindGroup.set}, {});
        }

        if (mask.indexBuffer != 0)
        {
            auto bufferHandle = reader.Read<BufferHandle>();
            auto& buffer      = renderContext.GetBuffer(bufferHandle);
            commandBuffer.bindIndexBuffer(buffer.vkBuffer, 0, vk::IndexType::eUint16);
        }

        if (mask.vertexBuffer != 0)
        {
            auto bufferHandle = reader.Read<BufferHandle>();
            auto& buffer      = renderContext.GetBuffer(bufferHandle);
            commandBuffer.bindVertexBuffers(0, {buffer.vkBuffer}, {0});
        }

        if (mask.dynamicBuffer != 0)
        {
            auto dynamicBufferHandle = reader.Read<DynamicBufferHandle>();
            auto& dynamicBuffer      = renderContext.GetDynamicBuffer(dynamicBufferHandle);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, graphicsPipeline.layout, 3, {dynamicBuffer.set}, {0});
        }

        if (mask.indexOffset != 0)
        {
            indexOffset = reader.Read<uint32_t>();
        }

        if (mask.vertexOffset != 0)
        {
            vertexOffset = reader.Read<uint32_t>();
        }

        if (mask.instanceOffset != 0)
        {
            instanceOffset = reader.Read<uint32_t>();
        }

        if (mask.instanceCount != 0)
        {
            instanceCount = reader.Read<uint32_t>();
        }

        if (mask.dynamicBufferOffset != 0)
        {
            dynamicBufferOffset = reader.Read<uint32_t>();
        }

        if (mask.indexCount != 0)
        {
            indexCount = reader.Read<uint32_t>();
        }

        if (mask.indexCount != 0)
        {
            commandBuffer.drawIndexed(indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
        }
        else
        {
            commandBuffer.draw(indexCount, instanceCount, vertexOffset, instanceOffset);
        }
    }
}
} // namespace gore::renderer