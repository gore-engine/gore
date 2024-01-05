#pragma once

#include "Core/System.h"

#include "Rendering/Vulkan/VulkanInstance.h"
#include "Rendering/Vulkan/VulkanDevice.h"
#include "Rendering/Vulkan/VulkanSurface.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanShader.h"
#include "Rendering/Vulkan/VulkanPipeline.h"

namespace gore
{

class Window;

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
    VulkanInstance* m_VulkanInstance;
    VulkanDevice* m_VulkanDevice;

    VulkanSurface* m_VulkanSurface;
    VulkanSwapchain* m_VulkanSwapchain;

    std::vector<VulkanSemaphore*> m_RenderFinishedSemaphores;

    VulkanShader* m_CubeShader;
    VulkanPipeline* m_CubePipeline;
};

} // namespace gore
