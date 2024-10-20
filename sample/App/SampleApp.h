#pragma once

#include "Core/App.h"
#include "Scene/Scene.h"

#include "Rendering/GraphicsCaps.h"
#include "Rendering/RenderContext.h"

#include "Scripts/Rendering/PerDrawData.h"
#include "Scripts/Rendering/GlobalData.h"

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
    // Temporary RenderPassDesc for pipeline creation
    void CreateRenderPassDesc();
    void CreateUnifiedGlobalDynamicBuffer();
    void CreateGlobalBindGroup();
    void CreatePipelines();

private:
    void UpdateFPSText(float deltaTime);

    gore::gfx::GraphicsCaps m_GraphicsCaps;

    struct SampleRenderPass
    {
        RenderPassDesc forwardPassDesc;
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
