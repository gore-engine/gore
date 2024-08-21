#pragma once

#include "Core/System.h"

#include "Graphics/Graphics.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "GraphicsCaps.h"
#include "RenderContext.h"
#include "CommandRing.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <functional>
#include <deque>

#define RPS_VK_RUNTIME 1
#include "rps/rps.h"

#include "RenderPipelineShader/RpsSytem.h"

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

    RenderContext& GetRenderContext() const { return *m_RenderContext; }

    void OnResize(Window* window, int width, int height);

private:
    // Imgui
    void InitImgui();
    void ShutdownImgui();

    ImGui_ImplVulkanH_Window m_ImguiWindowData;
    vk::raii::DescriptorPool m_ImguiDescriptorPool;

private:
    static void RecordDebugMarker(void* pUserContext, const RpsRuntimeOpRecordDebugMarkerArgs* pArgs);
    static void SetDebugName(void* pUserContext, const RpsRuntimeOpSetDebugNameArgs* pArgs);

    bool IsRpsReady() const { return m_RpsSystem != nullptr && m_RpsSystem->IsValid(); }

    void CreateRpsRuntimeDeivce();
    void DestroyRpsRuntimeDevice();

    void RunRpsSystem();

    void UpdateRenderGraph();
    RpsResult ExecuteRenderGraph();

    CommandRingElement RequestRpsNextCommandElement(RpsQueueType queueType, bool cyclePool = false, const vk::CommandBufferInheritanceInfo* pInheritanceInfo = nullptr);

    uint64_t CalcGuaranteedCompletedFrameindexForRps() const;

    static void DrawTriangle(const RpsCmdCallbackContext* pContext);
private:
    std::unique_ptr<RenderContext> m_RenderContext;

    // Instance
    Instance m_Instance;

    // Device
    Device m_Device;

    // Surface & Swapchain
    Swapchain m_Swapchain;

    GraphicsPipelineHandle m_CubePipelineHandle;
    GraphicsPipelineHandle m_UnLitPipelineHandle;
    GraphicsPipelineHandle m_TrianglePipelineHandle;
    GraphicsPipelineHandle m_QuadPipelineHandle;
    
    struct FrameFences
    {
        Fence     renderCompleteFence;
        Semaphore renderCompleteSemaphore;
        Semaphore imageAcquiredSemaphore;
    };
    std::vector<FrameFences>        m_frameFences;
    
    std::unique_ptr<RpsSytem> m_RpsSystem;
    std::vector<Semaphore> m_queueSemaphores;
    Semaphore m_pendingPresentSemaphore;
    uint32_t m_rpsQueueIndexToVkQueueFamilyMap[RPS_QUEUE_COUNT];
    std::vector<std::vector<RpsCommandPool>> m_cmdPools[RPS_QUEUE_COUNT];
    std::mutex m_cmdListMutex;
    std::vector<CommandBuffer> m_cmdBufsToSubmit;

    uint32_t m_FrameCounter;
    // Queue
    // Graphics, Compute, Transfer
    vk::Queue m_GpuQueues[RPS_QUEUE_COUNT];
    uint32_t m_GpuQueueFamilyIndices[RPS_QUEUE_COUNT];
    
    vk::raii::Queue m_PresentQueue;
    uint32_t m_PresentQueueFamilyIndex;

    // Command Pool & Command Buffer
    std::unique_ptr<CommandRing> m_GraphicsCommandRing;
    std::unique_ptr<CommandRing> m_GpuCommandRings[RPS_QUEUE_COUNT];
    std::unique_ptr<CommandRing> m_SecondaryCommandRing[RPS_QUEUE_COUNT];

    BindLayout m_GlobalBindLayout;
    BindGroupHandle m_GlobalBindGroup;
    BufferHandle m_GlobalConstantBuffer;

    // Material Descriptors
    BindLayout m_UVQuadBindLayout;
    BindGroupHandle m_UVQuadBindGroup;

    BufferHandle m_DynamicUniformBuffer;
    
    DynamicBufferHandle m_DynamicBufferHandle;

    TextureHandle m_UVCheckTextureHandle;
    SamplerHandle m_UVCheckSamplerHandle;

    // Depth buffer
    vk::Image m_DepthImage;
    VmaAllocation m_DepthImageAllocation;
    vk::raii::ImageView m_DepthImageView;

    DeletionQueue m_RenderDeletionQueue;

    GraphicsCaps m_GraphicsCaps;

private:
    void UploadPerframeGlobalConstantBuffer(uint32_t imageIndex);

    void CreateInstance();
    void CreateDevice();
    void CreateSurface();
    void CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height);
    void CreateDepthBuffer();
    void CreateGlobalDescriptorSets();
    void CreateUVQuadDescriptorSets();
    void CreateDynamicUniformBuffer();
    void CreatePipeline();
    void CreateTextureObjects();
    void GetQueues();
    
    [[nodiscard]] const PhysicalDevice& GetBestDevice(const std::vector<PhysicalDevice>& devices) const;
};

} // namespace gore
