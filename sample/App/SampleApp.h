#pragma once

#include "Core/App.h"
#include "Scene/Scene.h"

#include "Rendering/Pipeline.h"
#include "Rendering/BindLayout.h"
#include "Rendering/BindGroup.h"
#include "Rendering/DynamicBuffer.h"

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
    void CreateGlobalBindGroup();

    void Initialize() final;
    void Update() final;
    void Shutdown() final;

private:
    void CreateUnifiedGlobalDynamicBuffer();
    void CreateGlobalDescriptorSets();
    void CreatePipelines();

private:
    void UpdateFPSText(float deltaTime);

    struct SamplePipeline
    {
        GraphicsPipelineHandle blankPipeline;
        GraphicsPipelineHandle forwardPipeline;
        GraphicsPipelineHandle shadowPipeline;
        GraphicsPipelineHandle gbuffferPipeline;
    } pipelines;

    gore::gfx::DynamicBufferHandle m_UnifiedDynamicBuffer;

    gore::gfx::BindLayout m_GlobalBindLayout;
    gore::gfx::BindGroupHandle m_GlobalBindGroup;
    gore::gfx::BufferHandle m_GlobalConstantBuffer;
private:
    gore::Scene* scene;
};
