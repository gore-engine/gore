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
    RpsResult ExecuteRenderGraph(
        uint32_t frameIndex,
        RpsRenderGraph hRenderGraph,
        bool bWaitSwapChain = true,
        bool frameEnd       = true);
    CommandRingElement RequestRpsNextCommandElement(RpsQueueType queueType, bool cyclePool = false, const vk::CommandBufferInheritanceInfo* pInheritanceInfo = nullptr);

    struct ActiveCommandList
    {
        uint32_t backBufferIndex;
        uint32_t queueIndex;
        uint32_t poolIndex;
        vk::CommandBuffer cmdBuf;
        vk::CommandPool cmdPool;

        operator vk::CommandBuffer() const
        {
            return cmdBuf;
        }
    };

    void WaitForSwapChainBuffer();
    void WaitForGpuIdle();

    void PrepareSwapChain();
    void PresentSwapChain();

    ActiveCommandList BeginCmdList(RpsQueueType queueIndex, const vk::CommandBufferInheritanceInfo* pInheritanceInfo = nullptr);
    void SubmitCmdLists(
        ActiveCommandList* pCmdLists,
        uint32_t numCmdLists,
        bool frameEnd,
        uint32_t waitSemaphoreCount           = 0,
        const uint32_t* pWaitSemaphoreIndices = nullptr,
        uint32_t signalSemaphoreIndex         = UINT32_MAX,
        bool bWaitSwapChain                   = false);
    void EndCmdList(ActiveCommandList& cmdList);
    void RecycleCmdList(ActiveCommandList& cmdList);

    void ResetCommandPools();

    void ReserveSemaphores(uint32_t numSyncs);

    uint64_t CalcGuaranteedCompletedFrameindexForRps() const;

    static void DrawTriangleWithRPSWrapper(const RpsCmdCallbackContext* pContext);
    void DrawTriangle(vk::CommandBuffer commandBuffer);
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
        vk::Fence     renderCompleteFence;
        vk::Semaphore renderCompleteSemaphore;
    };
    std::vector<FrameFences>        m_frameFences;
    
    std::unique_ptr<RpsSytem> m_RpsSystem;
    std::vector<vk::Semaphore> m_queueSemaphores;
    vk::Semaphore m_pendingPresentSemaphore;
    std::vector<std::vector<RpsCommandPool>> m_cmdPools[RPS_QUEUE_COUNT];
    std::mutex m_cmdListMutex;
    std::vector<vk::CommandBuffer> m_cmdBufsToSubmit;
    std::vector<vk::Semaphore> m_imageAcquiredSemaphores;

    uint32_t m_FrameCounter;
    uint32_t m_backBufferIndex;
    uint32_t m_swapChainImageSemaphoreIndex;
    uint32_t m_pendingAcqImgSemaphoreIndex;

    // Queue
    // Graphics, Compute, Transfer
    vk::Queue m_GpuQueues[RPS_QUEUE_COUNT];
    uint32_t m_GpuQueueFamilyIndices[RPS_QUEUE_COUNT];
    
    vk::Queue m_PresentQueue;
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
