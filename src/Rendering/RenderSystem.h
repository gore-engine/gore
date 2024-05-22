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

using namespace gfx;

class Window;

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void PushFunction(std::function<void()>&& function)
    {
        deletors.push_back(function);
    }

    void Flush()
    {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call functors
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
    Instance m_Instance;

    // Device
    Device m_Device;

    // Surface & Swapchain
    Swapchain m_Swapchain;

    GraphicsPipelineHandle m_CubePipelineHandle;
    GraphicsPipelineHandle m_TrianglePipelineHandle;
    GraphicsPipelineHandle m_QuadPipelineHandle;
    
    // Pipeline
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::PipelineLayout m_BlankPipelineLayout;

    // Queue
    vk::raii::Queue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamilyIndex;
    vk::raii::Queue m_PresentQueue;
    uint32_t m_PresentQueueFamilyIndex;

    // Command Pool & Command Buffer
    CommandPool m_CommandPool;

    // Global Descriptors
    vk::raii::DescriptorPool m_GlobalDescriptorPool;
    vk::raii::DescriptorSetLayout m_GlobalDescriptorSetLayout;
    std::vector<vk::raii::DescriptorSet> m_GlobalDescriptorSets;

    std::vector<BufferHandle> m_GlobalConstantBuffers;
    
    // Material Descriptors
    vk::DescriptorPool m_MaterialDescriptorPool;
    vk::DescriptorSetLayout m_UVQuadDescriptorSetLayout;
    vk::DescriptorSet m_UVQuadDescriptorSet;

    TextureHandle m_UVCheckTextureHandle;
    SamplerHandle m_UVCheckSamplerHandle;
    vk::ImageView m_UVCheckImageView;

    // Synchronization
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_InFlightFences;

    // Depth buffer
    vk::Image m_DepthImage;
    VmaAllocation m_DepthImageAllocation;
    vk::raii::ImageView m_DepthImageView;

    BufferHandle m_IndexBufferHandle;
    BufferHandle m_VertexBufferHandle;

    DeletionQueue m_RenderDeletionQueue;

private:
    void UploadPerframeGlobalConstantBuffer(uint32_t imageIndex);

    void CreateInstance();
    void CreateDevice();
    void CreateSurface();
    void CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height);
    void CreateDepthBuffer();
    void CreateVertexBuffer();
    void CreateGlobalDescriptorSets();
    void CreateUVQuadDescriptorSets();
    void UpdateUVQuadDescriptorSets();
    void CreatePipeline();
    void CreateTextureObjects();
    void GetQueues();
    void CreateSynchronization();

    TextureHandle CreateTextureHandle(const std::string& name);

    [[nodiscard]] const PhysicalDevice& GetBestDevice(const std::vector<PhysicalDevice>& devices) const;
};

} // namespace gore
