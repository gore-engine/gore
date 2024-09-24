#pragma once

#include "Prefix.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindGroup.h"
#include "Rendering/Buffer.h"
#include "Rendering/DynamicBuffer.h"

#include "Rendering/Components/Material.h"

#include "Utilities/Hash/StdHash.h"

#include <vector>

namespace gore
{
class GameObject;
} // namespace gore

namespace gore::renderer
{
using namespace gore::gfx;

struct Pass;
class MeshRenderer;

// TODO: This is a temporary solution to use fixed pass names, we could use a more generic approach
struct DrawCreateInfo
{
    std::string passName = "";
    AlphaMode alphaMode  = AlphaMode::Opaque;
};

struct DrawCacheKey
{
    std::string passName = "";
    AlphaMode alphaMode  = AlphaMode::Opaque;

    bool operator==(const DrawCacheKey& other) const
    {
        return passName == other.passName && alphaMode == other.alphaMode;
    }

    bool operator!=(const DrawCacheKey& other) const
    {
        return !(*this == other);
    }

    bool operator<(const DrawCacheKey& other) const
    {
        if (passName != other.passName)
            return passName < other.passName;

        return alphaMode < other.alphaMode;
    }
};

struct Draw
{
    GraphicsPipelineHandle shader     = {};
    BindGroupHandle bindGroup[3]      = {};
    DynamicBufferHandle dynamicBuffer = {};
    BufferHandle vertexBuffer         = {}; // BufferHandle vertexBuffers[3], VertexBuffer could be an array of buffers for instancing or multiple vertex buffers, but for now we only need one
    BufferHandle indexBuffer          = {};
    uint32_t indexCount               = 0;
    uint32_t indexOffset              = 0;
    uint32_t vertexCount              = 0;
    uint32_t vertexOffset             = 0;
    uint32_t instanceCount            = 0;
    uint32_t instanceOffset           = 0;
    uint32_t dynamicBufferOffset      = 0;

    inline uint32_t GetTriangleCount() const
    {
        return indexCount / 3;
    }
};

struct DrawSorter
{
    bool operator()(const Draw& a, const Draw& b) const
    {
        if (a.shader.index() != b.shader.index())
            return a.shader.index() < b.shader.index();

        if (a.bindGroup[0].index() != b.bindGroup[0].index())
            return a.bindGroup[0].index() < b.bindGroup[0].index();

        if (a.bindGroup[1].index() != b.bindGroup[1].index())
            return a.bindGroup[1].index() < b.bindGroup[1].index();

        if (a.bindGroup[2].index() != b.bindGroup[2].index())
            return a.bindGroup[2].index() < b.bindGroup[2].index();

        if (a.dynamicBuffer.index() != b.dynamicBuffer.index())
            return a.dynamicBuffer.index() < b.dynamicBuffer.index();

        if (a.dynamicBufferOffset != b.dynamicBufferOffset)
            return a.dynamicBufferOffset < b.dynamicBufferOffset;

        if (a.vertexBuffer.index() != b.vertexBuffer.index())
            return a.vertexBuffer.index() < b.vertexBuffer.index();

        if (a.indexBuffer.index() != b.indexBuffer.index())
            return a.indexBuffer.index() < b.indexBuffer.index();

        if (a.vertexOffset != b.vertexOffset)
            return a.vertexOffset < b.vertexOffset;

        if (a.indexOffset != b.indexOffset)
            return a.indexOffset < b.indexOffset;

        return false;
    }
};

void PrepareDrawDataAndSort(DrawCreateInfo& info, std::vector<GameObject*>& gameObjects, std::vector<Draw>& sortedDrawData);
bool MatchDrawFilter(const Pass& pass, const DrawCreateInfo& info);
} // namespace gore::renderer

namespace std
{
template <>
struct hash<gore::renderer::DrawCacheKey>
{
    size_t operator()(const gore::renderer::DrawCacheKey& key) const
    {
        size_t result = 0;
        gore::utils::hash_combine(result, key.passName);
        gore::utils::hash_combine(result, static_cast<uint8_t>(key.alphaMode));
        return result;
    }
};
} // namespace std
