#pragma once

#include "Core/App.h"
#include "Scene/Scene.h"

class SampleApp final : public gore::App
{
public:
    SampleApp(int argc, char** argv);
    ~SampleApp() final;

protected:
    void Initialize() final;
    void Update() final;
    void Render() final;
    void Shutdown() final;

private:
    void UpdateFPSText(float deltaTime);

private:
    gore::Scene* scene;
};
