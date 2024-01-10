#pragma once

#include "Utilities/Defines.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{

class RenderSystem;
class App;
class Window;

namespace gfx
{

class Context
{
public:
    Context(RenderSystem* system, App* app);
    ~Context();

    NON_COPYABLE(Context);

    void Initialize();
    void Update();
    void Shutdown();

    void OnResize(Window* window, int width, int height);

private:
    RenderSystem* m_RenderSystem;

};

} // namespace gfx

} // namespace gore
