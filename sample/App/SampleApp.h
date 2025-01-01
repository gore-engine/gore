#pragma once

#include "Core/App.h"
#include "Scene/Scene.h"

#include "Rendering/GraphicsCaps.h"
#include "Rendering/RenderContext.h"

#define RPS_VK_RUNTIME 1
#include "rps/rps.h"

#include "Rendering/RenderPipelineShader/RpsSytem.h"

using namespace gore;

class SampleApp final : public gore::App
{
public:
    SampleApp(int argc, char** argv);
    ~SampleApp() final;

protected:
    void PrepareGraphics();

    void Initialize() final;
    void Update() final;
    void Shutdown() final;

private:
    void Preupdate();
    void UpdateImpl();
    void PostUpdate();
    void PreRender();

    // Temporary RenderPassDesc for pipeline creation
    void InitializeRpsSystem();
    void CreateRenderPassDesc();
    void CreateUnifiedGlobalDynamicBuffer();
    void CreateGlobalBindGroup();
    void CreatePipelines();

    void CreateForwardPipeline();
    void CreateShadowmapPipeline();

    const int K_OBJECT_BATCH_COUNT = 128;

    void UpdateAllGameObjectsGPUPerObjectData();
    
    static void DrawTriangleWithRPSWrapper(const RpsCmdCallbackContext* pContext);
    static void ShadowmapPassWithRPSWrapper(const RpsCmdCallbackContext* pContext);
    static void ForwardOpaquePassWithRPSWrapper(const RpsCmdCallbackContext* pContext);
private:
    void UpdateFPSText(float deltaTime);

    gore::gfx::GraphicsCaps m_GraphicsCaps;

    struct SampleRenderPass
    {
        RenderPassDesc forwardPassDesc;
        RenderPassDesc shadowPassDesc;
    } renderPasses;

    struct SamplePipeline
    {
        GraphicsPipelineHandle blankPipeline;
        GraphicsPipelineHandle forwardPipeline;
        GraphicsPipelineHandle shadowPipeline;
        GraphicsPipelineHandle gbuffferPipeline;
    } pipelines;

    gore::gfx::BufferHandle m_UnifiedDynamicBuffer;
    gore::gfx::DynamicBufferHandle m_UnifiedDynamicBufferHandle;

    gore::gfx::BindLayout m_GlobalBindLayout;
    gore::gfx::BindGroupHandle m_GlobalBindGroup;
    gore::gfx::BufferHandle m_GlobalConstantBuffer;
private:
    gore::Scene* scene;
};
