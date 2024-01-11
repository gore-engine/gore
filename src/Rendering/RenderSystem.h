#pragma once

#include "Core/System.h"

#include "Graphics/Graphics.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

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
    // Instance
    gfx::Instance m_Instance;

    // Device
    gfx::Device m_Device;

    // Surface & Swapchain
    gfx::Swapchain m_Swapchain;

    // Shader
    vk::raii::ShaderModule m_CubeVertexShader;
    std::string m_CubeVertexShaderEntryPoint;
    vk::raii::ShaderModule m_CubeFragmentShader;
    std::string m_CubeFragmentShaderEntryPoint;

    // Render pass
    vk::raii::RenderPass m_RenderPass;

    // Pipeline
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline m_Pipeline;

    // Framebuffer
    std::vector<vk::raii::Framebuffer> m_Framebuffers;

    // Queue
    vk::raii::Queue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamilyIndex;
    vk::raii::Queue m_PresentQueue;
    uint32_t m_PresentQueueFamilyIndex;

    // Command Pool & Command Buffer
    std::vector<vk::raii::CommandPool> m_CommandPools;
    std::vector<vk::raii::CommandBuffer> m_CommandBuffers;

    // Synchronization
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_InFlightFences;

    // Depth buffer
    vk::Image m_DepthImage;
    VmaAllocation m_DepthImageAllocation;
    vk::raii::ImageView m_DepthImageView;

private:
    void CreateDepthBuffer();
    void LoadShader(const std::string& name, const std::string& vertexEntryPoint, const std::string& fragmentEntryPoint);
    void CreateRenderPass();
    void CreatePipeline();
    void CreateFramebuffers();
    void GetQueues();
    void CreateCommandPools();
    void CreateSynchronization();

    [[nodiscard]] const gfx::PhysicalDevice& GetBestDevice(const std::vector<gfx::PhysicalDevice>& devices) const;
};

} // namespace gore
