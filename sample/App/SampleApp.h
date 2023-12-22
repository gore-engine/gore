#pragma once

#include "Core/App.h"

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

    void OnWindowResize(int width, int height) final;

private:
    void UpdateFPSText(float deltaTime);
};
