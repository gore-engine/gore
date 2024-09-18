#pragma once

#include "Core/App.h"
#include "Scene/Scene.h"

#include "Rendering/Pipeline.h"

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
    void UpdateFPSText(float deltaTime);

    struct SamplePipeline
    {
        GraphicsPipelineHandle blankPipeline;
        GraphicsPipelineHandle forwardPipeline;
        GraphicsPipelineHandle shadowPipeline;
        GraphicsPipelineHandle gbuffferPipeline;
    } pipelines;

private:
    gore::Scene* scene;
};
