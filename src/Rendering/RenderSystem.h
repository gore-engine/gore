#pragma once

#include "Core/System.h"

#include "Graphics/Graphics.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "RenderContext.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <functional>
#include <deque>

namespace gore
{

class Window;

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); //call functors
        }

        deletors.clear();
    }
};

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
    // Imgui
    void InitImgui();
    void ShutdownImgui();
    
    ImGui_ImplVulkanH_Window m_ImguiWindowData;
    vk::raii::DescriptorPool m_ImguiDescriptorPool;
private:
    std::unique_ptr<RenderContext> m_RenderContext;

    // Instance
    gfx::Instance m_Instance;

    // Device
    gfx::Device m_Device;

    // Surface & Swapchain
    gfx::Swapchain m_Swapchain;

    // Shader
    ShaderModuleHandle m_CubeVertexShaderHandle;
    ShaderModuleHandle m_CubeFragmentShaderHandle;

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
    gfx::CommandPool m_CommandPool;

    // Global Descriptors
    vk::raii::DescriptorPool m_GlobalDescriptorPool;
    vk::raii::DescriptorSetLayout m_GlobalDescriptorSetLayout;
    std::vector<vk::raii::DescriptorSet> m_GlobalDescriptorSets;

    std::vector<BufferHandle> m_GlobalConstantBuffers;

    // Synchronization
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_InFlightFences;

    // Depth buffer
    vk::Image m_DepthImage;
    VmaAllocation m_DepthImageAllocation;
    vk::raii::ImageView m_DepthImageView;


    BufferHandle m_IndexBufferHandle;

    vk::raii::Buffer m_VertexBuffer;
    vk::raii::DeviceMemory m_VertexBufferMemory;
    // vk::raii::Buffer m_IndexBuffer;
    // vk::raii::DeviceMemory m_IndexBufferMemory;

    DeletionQueue m_RenderDeletionQueue;

private:
    uint32_t FindMemoryType(uint32_t typeFilter, vk::PhysicalDeviceMemoryProperties memProperties, vk::MemoryPropertyFlags properties) const;

    void CreateInstance();
    void CreateDevice();
    void CreateSurface();
    void CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height);
    void CreateDepthBuffer();
    void CreateVertexBuffer();
    void LoadShader(const std::string& name, const std::string& vertexEntryPoint, const std::string& fragmentEntryPoint);
    void CreateRenderPass();
    void CreateGlobalDescriptorSets();
    void CreatePipeline();
    void CreateFramebuffers();
    void GetQueues();
    void CreateSynchronization();

    [[nodiscard]] const gfx::PhysicalDevice& GetBestDevice(const std::vector<gfx::PhysicalDevice>& devices) const;
};

} // namespace gore
