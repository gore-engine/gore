#pragma once

#include "Core/System.h"

namespace gore
{

class Window;

namespace graphics
{
class Context;
}

class RenderSystem final : System
{
public:
    explicit RenderSystem(App* app);
    ~RenderSystem() override;

    NON_COPYABLE(RenderSystem);

    void Initialize() override;
    void Update() override;
    void Shutdown() override;

    void OnResize(Window* window, int width, int height);

private:
    graphics::Context* m_Context;
};

} // namespace gore
