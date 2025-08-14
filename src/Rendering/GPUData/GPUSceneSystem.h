#pragma once

#include "Rendering/RenderContext.h"

#include "GPUScene.h"

namespace gore::renderer
{
class GPUSceneSystem
{
public:
    explicit GPUSceneSystem(RenderContext* context);
    ~GPUSceneSystem();

    void CreateGPUSceneData();
    void CleanupGPUSceneData();

    void BeginUpdateSceneData();
    void EndUpdateSceneData();

private:
    void UpdateMaterialData();
    void UpdateGeometryData();
    void UpdateInstanceInfoData();

private:
    struct GPUSceneData
    {
        BufferHandle materialBuffer;
        BufferHandle geometryBuffer;
        BufferHandle instanceBuffer;
    } m_GPUSceneBuffer;

    GPUScene m_GPUSceneData;

    RenderContext* m_Context;
};
} // namespace gore::renderer