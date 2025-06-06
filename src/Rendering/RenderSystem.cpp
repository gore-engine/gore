#include "Prefix.h"

#include "RenderSystem.h"

#include "RenderContext.h"

#include "Graphics/Utils.h"
#include "Core/App.h"
#include "Core/Time.h"
#include "FileSystem/FileSystem.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Constants.h"
#include "Windowing/Window.h"
#include "Scene/Scene.h"
#include "Object/Camera.h"
#include "Object/GameObject.h"

#include "Rendering/Components/MeshRenderer.h"
#include "Utilities/GLTFLoader.h"
#include "Utilities/Math/MathHelpers.h"

#include "Rendering/AutoRenderPass.h"
#include "RenderContextHelper.h"
#include "Rendering/Components/Light.h"
#include "Rendering/GPUData/PerDrawData.h"
#include "Rendering/GPUData/PerFrameData.h"

#include "Profiler/microprofile.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>

MICROPROFILE_DEFINE(g_RenderSystemInit, "System", "RenderSystemInit", MP_AUTO);
MICROPROFILE_DEFINE(g_PrepareDrawData, "RenderSystemLoop", "PrepareDrawData", MP_BLUE);
MICROPROFILE_DEFINE(g_RenderGraphUpdate, "RenderSystemLoop", "RenderGraphUpdate", MP_BLUE);
MICROPROFILE_DEFINE(g_ExecuteRenderGraph, "RenderSystemLoop", "ExecuteRenderGraph", MP_BLUE);

namespace gore
{

static RenderSystem* g_RenderSystem = nullptr;

RenderSystem::RenderSystem(gore::App* app) :
    System(app),
    m_GraphicsCaps(),
    // Instance
    m_Instance(app),
    // Device
    m_Device(),
    // Surface & Swapchain
    m_Swapchain(),
    // Queue
    m_GpuQueues(),
    m_GpuQueueFamilyIndices(),
    m_PresentQueue(nullptr),
    m_PresentQueueFamilyIndex(0),
    // Depth Buffer
    m_DepthImage(nullptr),
    m_DepthImageAllocation(VK_NULL_HANDLE),
    m_DepthImageView(nullptr),
    // Imgui
    m_ImguiWindowData(),
    m_ImguiDescriptorPool(nullptr),
    // Utils
    m_RenderDeletionQueue(),
    m_FrameCounter(0),
    m_backBufferIndex(0),
    m_swapChainImageSemaphoreIndex(0),
    m_pendingAcqImgSemaphoreIndex(UINT32_MAX),
    // Draw Data
    m_DrawData()
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    m_RenderContext.release();

    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    MICROPROFILE_SCOPE(g_RenderSystemInit);

    Window* window = m_App->GetWindow();
    int width, height;
    window->GetSize(&width, &height);

    std::vector<PhysicalDevice> physicalDevices = m_Instance.GetPhysicalDevices();
    m_Device = Device(GetBestDevice(physicalDevices));

    m_FrameCounter = 0;
    m_backBufferIndex = 0;

	VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Device.Get());

    CreateRpsRuntimeDeivce();

    uint32_t swapchainCount = 3;

    m_Swapchain = m_Device.CreateSwapchain(window->GetNativeHandle(), swapchainCount, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    m_Device.SetName(m_Swapchain.Get(), "Main Swapchain");

    InitVulkanGraphicsCaps(m_GraphicsCaps, m_Instance, m_Device);

    RenderContextCreateInfo renderContextCreateInfo = {};
    renderContextCreateInfo.device = &m_Device;
    renderContextCreateInfo.flags = PSO_CREATE_FLAG_PREFER_RPS;

    m_RenderContext = std::make_unique<RenderContext>(renderContextCreateInfo);
    m_RenderContext->PrepareRendering();

    m_frameFences.resize(swapchainCount);
    for (uint32_t i = 0; i < swapchainCount; i++)
    {
        m_frameFences[i].renderCompleteFence = (*m_Device.Get()).createFence({ vk::FenceCreateFlagBits::eSignaled });
    }
    
    m_imageAcquiredSemaphores.resize(swapchainCount + 1);
    for (uint32_t i = 0; i < swapchainCount + 1; i++)
    {
        m_imageAcquiredSemaphores[i] = (*m_Device.Get()).createSemaphore({});
    }

    m_RenderDeletionQueue.PushFunction([this]()
    {
        for (auto& frameFence : m_frameFences)
        {
            (*m_Device.Get()).destroyFence(frameFence.renderCompleteFence);            
        }
        for (auto& imageAcquiredSemaphore : m_imageAcquiredSemaphores)
        {
            (*m_Device.Get()).destroySemaphore(imageAcquiredSemaphore);
        }
    });
    
    CreateDefaultResources();

    CreateDepthBuffer();
    
    CreateTextureObjects();

    CreateGlobalDescriptorSets();
    CreateMaterialDescriptorSets();
    CreateShadowPassObject();
    CreateUVQuadDescriptorSets();
    CreateDynamicUniformBuffer();
    CreateRpsPipelines();
    CreatePipeline();
    GetQueues();

    InitImgui();
}

static void PrepareDrawStreamByDrawInfo(std::unordered_map<DrawKey, DrawStream>& map, DrawCreateInfo info, std::vector<GameObject*>& gameObjects, Material* overrideMaterial)
{
    DrawKey key = {};
    key.passName = info.passName;
    key.alphaMode = info.alphaMode;

    std::vector<Draw> sortedDrawData;
    PrepareDrawDataAndSort(info, gameObjects, sortedDrawData, overrideMaterial);

    DrawStream drawStream;
    CreateDrawStreamFromDrawData(sortedDrawData, drawStream);

    map[key] = drawStream;
}

void RenderSystem::PrepareDrawData()
{
    MICROPROFILE_SCOPE(g_PrepareDrawData);

    DrawCreateInfo info = {};
    info.passName = "ForwardPass";
    info.alphaMode = AlphaMode::Opaque;

    DrawCreateInfo shadowInfo = {};
    shadowInfo.passName = "ShadowCaster";
    shadowInfo.alphaMode = AlphaMode::Opaque;

    std::vector<GameObject*> gameObjects = Scene::GetActiveScene()->GetGameObjects();
    m_DrawData.clear();

    PrepareDrawStreamByDrawInfo(m_DrawData, info, gameObjects, &m_RpsMaterial.forward);
    PrepareDrawStreamByDrawInfo(m_DrawData, shadowInfo, gameObjects, &m_RpsMaterial.forward);
}

void RenderSystem::Update()
{
    PrepareDrawData();

    return RunRpsSystem();
}

void RenderSystem::Shutdown()
{
    WaitForGpuIdle();

    if (m_DepthImage != nullptr)
    {
        m_DepthImageView = nullptr;

        vmaDestroyImage(m_Device.GetVmaAllocator(), m_DepthImage, m_DepthImageAllocation);
    }
    
    for (uint32_t queueId = RPS_QUEUE_GRAPHICS; queueId < RPS_QUEUE_COUNT; queueId++)
    {
        for (auto& cmdPools : m_cmdPools[queueId])
        {
            for (auto& cmdPool : cmdPools)
            {
                (*m_Device.Get()).destroyCommandPool(cmdPool.cmdPool);
            }
        }
    }

    DestroyRpsRuntimeDevice();

    m_RenderDeletionQueue.Flush();

    m_RenderContext->Clear();

    ShutdownImgui();
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
    vk::Extent2D surfaceExtent = m_Swapchain.GetExtent();
    if (surfaceExtent.width == static_cast<uint32_t>(width) && surfaceExtent.height == static_cast<uint32_t>(height))
        return;
    
    WaitForGpuIdle();
    if (m_DepthImage != nullptr)
    {
        m_DepthImageView = nullptr;
        vmaDestroyImage(m_Device.GetVmaAllocator(), m_DepthImage, m_DepthImageAllocation);
    }

    m_Swapchain.Recreate(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    CreateDepthBuffer();

    // ImGui recreate
    DestroyImGuiFramebuffer();
    CreateImGuiFramebuffer();
}

void RenderSystem::DrawRenderer(DrawKey key, vk::CommandBuffer cmd, GraphicsPipelineHandle overridePipeline)
{
    if (m_DrawData.find(key) == m_DrawData.end())
        return;

    ScheduleDrawStream(*m_RenderContext, m_DrawData[key], cmd);        
}

void RenderSystem::CreateImGuiFramebuffer()
{
    assert(m_ImGuiObjects.renderPass != VK_NULL_HANDLE);

    vk::FramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.renderPass = m_ImGuiObjects.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = m_Swapchain.GetExtent().width;
    framebufferInfo.height = m_Swapchain.GetExtent().height;
    framebufferInfo.layers = 1;

    assert(m_ImGuiObjects.framebuffers.empty());
    m_ImGuiObjects.framebuffers.resize(m_Swapchain.GetImageCount());

    for (uint32_t i = 0; i < m_Swapchain.GetImageCount(); i++)
    {
        framebufferInfo.pAttachments = &(*m_Swapchain.GetImageViews()[i]);
        m_ImGuiObjects.framebuffers[i] = (*m_Device.Get()).createFramebuffer(framebufferInfo);
    }
}

void RenderSystem::DestroyImGuiFramebuffer()
{
    for (auto& framebuffer : m_ImGuiObjects.framebuffers)
    {
        (*m_Device.Get()).destroyFramebuffer(framebuffer);
    }
    m_ImGuiObjects.framebuffers.clear();
}

void RenderSystem::InitImgui()
{
    //1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
    vk::DescriptorPoolSize pool_sizes[] =
    {
        { vk::DescriptorType::eSampler, 1000 },
        { vk::DescriptorType::eCombinedImageSampler, 1000 },
        { vk::DescriptorType::eSampledImage, 1000 },
        { vk::DescriptorType::eStorageImage, 1000 },
        { vk::DescriptorType::eUniformTexelBuffer, 1000 },
        { vk::DescriptorType::eStorageTexelBuffer, 1000 },
        { vk::DescriptorType::eUniformBuffer, 1000 },
        { vk::DescriptorType::eStorageBuffer, 1000 },
        { vk::DescriptorType::eUniformBufferDynamic, 1000 },
        { vk::DescriptorType::eStorageBufferDynamic, 1000 },
        { vk::DescriptorType::eInputAttachment, 1000 }
    };

    vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    vk::DescriptorPoolCreateInfo pool_info(flags, 1000, 11, pool_sizes);

	m_ImguiDescriptorPool = m_Device.Get().createDescriptorPool(pool_info);

	// 2: initialize imgui library

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//this initializes imgui for SDL
	ImGui_ImplGlfw_InitForVulkan(m_App->GetWindow()->Get(), true);

    //this initialize renderPass for ImGui
    vk::AttachmentDescription attachmentDesc = {};
    attachmentDesc.format = m_Swapchain.GetFormat().format;
    attachmentDesc.samples = vk::SampleCountFlagBits::e1;
    attachmentDesc.loadOp = vk::AttachmentLoadOp::eLoad;
    attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
    attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachmentDesc.initialLayout = vk::ImageLayout::eUndefined;
    attachmentDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &attachmentDesc;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDesc;
    m_ImGuiObjects.renderPass = (*m_Device.Get()).createRenderPass(renderPassInfo);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = *m_Device.GetInstance()->Get();
	init_info.PhysicalDevice = *m_Device.GetPhysicalDevice().Get();
	init_info.Device = *m_Device.Get();
	init_info.Queue = m_GpuQueues[RPS_QUEUE_GRAPHICS];
	init_info.DescriptorPool = *m_ImguiDescriptorPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	ImGui_ImplVulkan_Init(&init_info, m_ImGuiObjects.renderPass);

    CreateImGuiFramebuffer();
}

void RenderSystem::ShutdownImgui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_ImguiDescriptorPool.clear();

    (*m_Device.Get()).destroyRenderPass(m_ImGuiObjects.renderPass);
    
    DestroyImGuiFramebuffer();
}

void RenderSystem::RecordDebugMarker(void* pUserContext, const RpsRuntimeOpRecordDebugMarkerArgs* pArgs)
{
    auto cmdBuffer = rpsVKCommandBufferFromHandle(pArgs->hCommandBuffer);
    auto renderSystem = static_cast<RenderSystem*>(pUserContext);

    CommandBuffer cmd = { nullptr, cmdBuffer };
    switch (pArgs->mode)
    {
        case RPS_RUNTIME_DEBUG_MARKER_BEGIN:
        renderSystem->m_RenderContext->BeginDebugLabel(cmd, pArgs->text, 1.0f, 0.0f, 0.0f);
        break;
        case RPS_RUNTIME_DEBUG_MARKER_END:
        renderSystem->m_RenderContext->EndDebugLabel(cmd);
        break;
        case RPS_RUNTIME_DEBUG_MARKER_LABEL:
        renderSystem->m_RenderContext->InsertDebugLabel(cmd, pArgs->text, 1.0f, 0.0f, 0.0f);
        break;
    }
}

void RenderSystem::SetDebugName(void* pUserContext, const RpsRuntimeOpSetDebugNameArgs* pArgs)
{
    auto renderSystem = static_cast<RenderSystem*>(pUserContext);

    uint64_t handle = reinterpret_cast<uint64_t>(static_cast<void*>(pArgs->hResource.ptr));
    VkObjectType objectType = (rps::ResourceDesc::IsBuffer(pArgs->resourceType)) ? VK_OBJECT_TYPE_BUFFER : VK_OBJECT_TYPE_IMAGE;

    Device& device = renderSystem->m_Device;

    device.SetName(handle, vk::ObjectType(objectType), pArgs->name);
}

void RenderSystem::CreateRpsRuntimeDeivce()
{
    using namespace gfx;

    RpsSytemCreateInfo createInfo = {};
    createInfo.device = *m_Device.Get();
    createInfo.physicalDevice = *m_Device.GetPhysicalDevice().Get();
    createInfo.pUserContext = this;
    createInfo.pfnRecordDebugMarker = &RecordDebugMarker;
    createInfo.pfnSetDebugName = &SetDebugName;

    m_RpsSystem = InitializeRpsSystem(createInfo);
    
    RpsRenderGraph& renderGraph = *m_RpsSystem->rpsRDG;

    AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "Shadowmap", &ShadowmapPassWithRPSWrapper, this));
    AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "ForwardOpaque", &ForwardOpaquePassWithRPSWrapper, this));
}

void RenderSystem::DestroyRpsRuntimeDevice()
{
    using namespace gfx;
    DestroyRpsSystem(m_RpsSystem);
}

void RenderSystem::RunRpsSystem()
{
    if (IsRpsReady() == false)
        return;

    UpdateRenderGraph();
        
    WaitForSwapChainBuffer();

    UpdateGlobalConstantBuffer();

    ResetPerFrameDescriptorPool();

    ResetCommandPools();

    ExecuteRenderGraph(m_FrameCounter, *m_RpsSystem->rpsRDG, true, false);

    StartImguiDraw();

    DrawImgui();

    EndImguiDraw(*m_RpsSystem->rpsRDG);

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.swapchainCount   = 1;
    presentInfo.pSwapchains      = &(*m_Swapchain.Get());
    presentInfo.pImageIndices    = &m_backBufferIndex;

    if (m_pendingPresentSemaphore != VK_NULL_HANDLE)
    {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &m_pendingPresentSemaphore;
        m_pendingPresentSemaphore = VK_NULL_HANDLE;
    }

    VK_CHECK_RESULT(m_PresentQueue.presentKHR(presentInfo));
    m_FrameCounter++;
}

void RenderSystem::UpdateRenderGraph()
{
    MICROPROFILE_SCOPE(g_RenderGraphUpdate);

    if (IsRpsReady() == false)
        return;

    uint32_t backBufferCount = m_Swapchain.GetImageCount();

    std::vector<RpsRuntimeResource> backBufferResources(backBufferCount);
    for (uint32_t i = 0; i < backBufferCount; i++)
    {
        backBufferResources[i].ptr = m_Swapchain.GetImages()[i];
    }

    RpsResourceDesc backBufferDesc   = {};
    backBufferDesc.type              = RPS_RESOURCE_TYPE_IMAGE_2D;
    backBufferDesc.temporalLayers    = uint32_t(m_Swapchain.GetImageCount());
    backBufferDesc.image.arrayLayers = 1;
    backBufferDesc.image.mipLevels   = 1;
    backBufferDesc.image.format      = rpsFormatFromVK((VkFormat)m_Swapchain.GetFormat().format);
    backBufferDesc.image.width       = m_Swapchain.GetExtent().width;
    backBufferDesc.image.height      = m_Swapchain.GetExtent().height;
    backBufferDesc.image.sampleCount = 1;

    RpsConstant argsData[2]                   = {&backBufferDesc};

    const uint64_t gpuCompletedFrameIndex = CalcGuaranteedCompletedFrameindexForRps();

    const RpsRuntimeResource* argResources[2] = {backBufferResources.data()};
    uint32_t argsCount                        = 1;

    RpsRenderGraphUpdateInfo updateInfo = {};
    updateInfo.frameIndex               = m_FrameCounter;
    updateInfo.gpuCompletedFrameIndex   = gpuCompletedFrameIndex;
    updateInfo.numArgs                  = argsCount;
    updateInfo.ppArgs                   = argsData;
    updateInfo.ppArgResources           = argResources;
    
    updateInfo.diagnosticFlags = RPS_DIAGNOSTIC_ENABLE_RUNTIME_DEBUG_NAMES;
    if (m_FrameCounter < m_Swapchain.GetImageCount())
    {
        updateInfo.diagnosticFlags = RPS_DIAGNOSTIC_ENABLE_ALL;
    }

    assert(_countof(argsData) == _countof(argResources));

    AssertIfRpsFailed(rpsRenderGraphUpdate(*m_RpsSystem->rpsRDG, &updateInfo));
}

RpsResult RenderSystem::ExecuteRenderGraph(
    uint32_t frameIndex,
    RpsRenderGraph hRenderGraph,
    bool bWaitSwapChain,
    bool frameEnd)
{
    MICROPROFILE_SCOPE(g_ExecuteRenderGraph);

    if (IsRpsReady() == false)
    {
        LOG_STREAM(FATAL) << "RPS system is not ready." << std::endl;
        return RpsResult::RPS_ERROR_UNSPECIFIED; 
    }
    
    RpsRenderGraphBatchLayout batchLayout = {};
    RpsResult result = rpsRenderGraphGetBatchLayout(*m_RpsSystem->rpsRDG, &batchLayout);
    if (RPS_FAILED(result))
    {
        return result;
    }
    ReserveSemaphores(batchLayout.numFenceSignals);

    for (uint32_t iBatch = 0; iBatch < batchLayout.numCmdBatches; iBatch++)
    {
        auto& batch = batchLayout.pCmdBatches[iBatch];

        ActiveCommandList cmdList = BeginCmdList(RpsQueueType(batch.queueIndex));

        RpsRenderGraphRecordCommandInfo recordInfo = {};

        recordInfo.hCmdBuffer    = rpsVKCommandBufferToHandle(cmdList.cmdBuf);
        recordInfo.pUserContext  = this;
        recordInfo.frameIndex    = frameIndex;
        recordInfo.cmdBeginIndex = batch.cmdBegin;
        recordInfo.numCmds       = batch.numCmds;

        if (g_DebugMarkers)
        {
            recordInfo.flags = RPS_RECORD_COMMAND_FLAG_ENABLE_COMMAND_DEBUG_MARKERS;
        }

        result = rpsRenderGraphRecordCommands(hRenderGraph, &recordInfo);
        if (RPS_FAILED(result))
            return result;

        EndCmdList(cmdList);

        SubmitCmdLists(&cmdList,
                       1,
                       frameEnd && ((iBatch + 1) == batchLayout.numCmdBatches),
                       batch.numWaitFences,
                       batchLayout.pWaitFenceIndices + batch.waitFencesBegin,
                       batch.signalFenceIndex,
                       bWaitSwapChain && (iBatch == 0)); // TODO - RPS to mark first access to swapchain image

        RecycleCmdList(cmdList);
    }

    if (batchLayout.numCmdBatches == 0)
    {
        SubmitCmdLists(nullptr, 0, frameEnd, 0, nullptr, UINT32_MAX, bWaitSwapChain);
    }

    return RPS_OK;
}

void RenderSystem::StartImguiDraw()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RenderSystem::DrawImgui()
{
    bool show = true;
    ImGui::ShowDemoWindow(&show);
}

void RenderSystem::EndImguiDraw(RpsRenderGraph renderGraph)
{
    ImGui::Render();

    RpsRenderGraphBatchLayout batchLayout = {};
    RpsResult result                      = rpsRenderGraphGetBatchLayout(renderGraph, &batchLayout);
    AssertIfRpsFailed(result);

    ActiveCommandList cmdList = BeginCmdList(RPS_QUEUE_GRAPHICS);

    vk::ImageMemoryBarrier graphToGuiBarrier = {};
    // TODO Better solution for srcAccessMask
    graphToGuiBarrier.srcAccessMask = {};
    graphToGuiBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    graphToGuiBarrier.oldLayout =
        m_FrameCounter < m_Swapchain.GetImageCount() ? vk::ImageLayout::eUndefined : vk::ImageLayout::ePresentSrcKHR;
    graphToGuiBarrier.newLayout                   = vk::ImageLayout::eColorAttachmentOptimal;
    graphToGuiBarrier.image                       = m_Swapchain.GetImages()[m_backBufferIndex];
    graphToGuiBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    graphToGuiBarrier.subresourceRange.layerCount = 1;
    graphToGuiBarrier.subresourceRange.levelCount = 1;

    cmdList.cmdBuf.pipelineBarrier(
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        {},
        0,
        nullptr,
        0,
        nullptr,
        1,
        &graphToGuiBarrier);

    vk::ClearValue clearColor = {};

    vk::RenderPassBeginInfo beginInfo = {};
    beginInfo.framebuffer             = m_ImGuiObjects.framebuffers[m_backBufferIndex];
    beginInfo.clearValueCount         = 1;
    beginInfo.pClearValues            = &clearColor;
    beginInfo.renderPass              = m_ImGuiObjects.renderPass;
    beginInfo.renderArea              = vk::Rect2D{{0, 0}, m_Swapchain.GetExtent()};

    cmdList.cmdBuf.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdList.cmdBuf);
    cmdList.cmdBuf.endRenderPass();

    EndCmdList(cmdList);

    SubmitCmdLists(&cmdList, 1, true);

    RecycleCmdList(cmdList);
}

void RenderSystem::WaitForSwapChainBuffer()
{
    m_swapChainImageSemaphoreIndex = (m_swapChainImageSemaphoreIndex + 1) % m_imageAcquiredSemaphores.size();

    VK_CHECK_RESULT((*m_Device.Get()).acquireNextImageKHR(*m_Swapchain.Get(), UINT64_MAX, m_imageAcquiredSemaphores[m_swapChainImageSemaphoreIndex], VK_NULL_HANDLE, &m_backBufferIndex));
    
    m_pendingAcqImgSemaphoreIndex = m_swapChainImageSemaphoreIndex;

    if ((m_FrameCounter % m_Swapchain.GetImageCount()) != m_backBufferIndex)
        m_FrameCounter = m_backBufferIndex;

    VK_CHECK_RESULT((*m_Device.Get()).waitForFences(1, &m_frameFences[m_backBufferIndex].renderCompleteFence, VK_TRUE, UINT64_MAX));
    VK_CHECK_RESULT((*m_Device.Get()).resetFences(1, &m_frameFences[m_backBufferIndex].renderCompleteFence));
}

void RenderSystem::WaitForGpuIdle()
{
    m_Device.WaitIdle();
}

void RenderSystem::PrepareSwapChain()
{    
    ActiveCommandList cmdList = BeginCmdList(RPS_QUEUE_GRAPHICS);

    vk::Image backBuffer = m_Swapchain.GetImages()[m_backBufferIndex];

    vk::ImageMemoryBarrier imageBarrier = {};
    imageBarrier.srcAccessMask               = {};
    imageBarrier.dstAccessMask               = {};
    imageBarrier.oldLayout                   = vk::ImageLayout::eUndefined;
    imageBarrier.newLayout                   = vk::ImageLayout::eColorAttachmentOptimal;
    imageBarrier.image                       = backBuffer;
    imageBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.levelCount = 1;

    std::vector<vk::ImageMemoryBarrier> imageBarriers = {imageBarrier};

    cmdList.cmdBuf.pipelineBarrier(
                            vk::PipelineStageFlagBits::eBottomOfPipe,
                            vk::PipelineStageFlagBits::eTopOfPipe,                          
                            {},
                            {},
                            {},
                            imageBarriers);

    EndCmdList(cmdList);

    SubmitCmdLists(&cmdList, 1, false);

    RecycleCmdList(cmdList);
}

void RenderSystem::PresentSwapChain()
{
    ActiveCommandList cmdList = BeginCmdList(RPS_QUEUE_GRAPHICS);

    vk::Image backBuffer = m_Swapchain.GetImages()[m_backBufferIndex];

    vk::ImageMemoryBarrier imageBarrier = {};
    imageBarrier.srcAccessMask               = vk::AccessFlagBits::eColorAttachmentWrite;
    imageBarrier.dstAccessMask               = vk::AccessFlagBits::eMemoryRead;
    imageBarrier.oldLayout                   = vk::ImageLayout::eColorAttachmentOptimal;
    imageBarrier.newLayout                   = vk::ImageLayout::ePresentSrcKHR;
    imageBarrier.image                       = backBuffer;
    imageBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.levelCount = 1;

    std::vector<vk::ImageMemoryBarrier> imageBarriers = {imageBarrier};

    cmdList.cmdBuf.pipelineBarrier(
                            vk::PipelineStageFlagBits::eColorAttachmentOutput,
                            vk::PipelineStageFlagBits::eAllCommands,                          
                            {},
                            {},
                            {},
                            imageBarriers);

    EndCmdList(cmdList);

    SubmitCmdLists(&cmdList, 1, true);

    RecycleCmdList(cmdList);
}

RenderSystem::ActiveCommandList RenderSystem::BeginCmdList(RpsQueueType queueIndex, const vk::CommandBufferInheritanceInfo* pInheritanceInfo)
{
    ActiveCommandList result = {};
    result.backBufferIndex   = m_backBufferIndex;
    result.queueIndex        = queueIndex;
    result.cmdPool           = VK_NULL_HANDLE;

    std::lock_guard<std::mutex> lock(m_cmdListMutex);

    if (m_cmdPools[queueIndex].size() <= m_Swapchain.GetImageCount())
    {
        m_cmdPools[queueIndex].resize(m_Swapchain.GetImageCount());
    }

    uint32_t freeIdx = 0;
    for (; freeIdx < m_cmdPools[queueIndex][m_backBufferIndex].size(); freeIdx++)
    {
        if (!m_cmdPools[queueIndex][m_backBufferIndex][freeIdx].inUse)
            break;
    }

    if (freeIdx == m_cmdPools[queueIndex][m_backBufferIndex].size())
    {
        vk::CommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.queueFamilyIndex          = m_GpuQueueFamilyIndices[queueIndex];

        RpsCommandPool newPool = {};
        newPool.cmdPool        = (*m_Device.Get()).createCommandPool(cmdPoolInfo);

        m_cmdPools[queueIndex][m_backBufferIndex].emplace_back(newPool);
    }

    RpsCommandPool* pPool = &m_cmdPools[queueIndex][m_backBufferIndex][freeIdx];
    pPool->inUse          = true;
    result.poolIndex      = freeIdx;
    result.cmdPool        = pPool->cmdPool;

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandBufferCount            = 1;
    allocInfo.commandPool                   = result.cmdPool;
    allocInfo.level                         = (pInheritanceInfo == nullptr) ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary;

    result.cmdBuf = (*m_Device.Get()).allocateCommandBuffers(allocInfo)[0];

    pPool->cmdBuffers.push_back(result.cmdBuf);

    vk::CommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.pInheritanceInfo           = pInheritanceInfo;
    cmdBeginInfo.flags                      = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    if (pInheritanceInfo)
        cmdBeginInfo.flags |= vk::CommandBufferUsageFlagBits::eRenderPassContinue;

    result.cmdBuf.begin(cmdBeginInfo);
    return result;
}

void RenderSystem::SubmitCmdLists(ActiveCommandList* pCmdLists, uint32_t numCmdLists, bool frameEnd, uint32_t waitSemaphoreCount, const uint32_t* pWaitSemaphoreIndices, uint32_t signalSemaphoreIndex, bool bWaitSwapChain)
{
    vk::CommandBuffer* pCmdBufs = nullptr;

    if (numCmdLists > 0)
    {
        pCmdBufs = &pCmdLists[0].cmdBuf;
        if (numCmdLists > 1)
        {
            m_cmdBufsToSubmit.resize(numCmdLists);
            for (uint32_t i = 0; i < m_cmdBufsToSubmit.size(); i++)
            {
                m_cmdBufsToSubmit[i] = pCmdLists[i].cmdBuf;
            }
            pCmdBufs = m_cmdBufsToSubmit.data();
        }
    }

    vk::PipelineStageFlags submitWaitStage = vk::PipelineStageFlagBits::eBottomOfPipe;

    uint32_t numWaitSemaphores                       = 0;
    vk::Semaphore waitSemaphores[RPS_MAX_QUEUES + 1] = {};

    // Wait for swapchain if there's a pending signal, and if user asked to wait or at frame end.
    if ((m_pendingAcqImgSemaphoreIndex != UINT32_MAX) && (bWaitSwapChain || frameEnd))
    {
        waitSemaphores[numWaitSemaphores] = m_imageAcquiredSemaphores[m_pendingAcqImgSemaphoreIndex];
        ++numWaitSemaphores;
        m_pendingAcqImgSemaphoreIndex = UINT32_MAX;
    }

    assert(waitSemaphoreCount <= RPS_MAX_QUEUES);

    for (uint32_t i = 0; (i < waitSemaphoreCount) && (i < RPS_MAX_QUEUES); i++)
    {
        assert(pWaitSemaphoreIndices[i] < m_queueSemaphores.size());
        waitSemaphores[numWaitSemaphores] = m_queueSemaphores[pWaitSemaphoreIndices[i]];
        ++numWaitSemaphores;
    }

    vk::Fence submitFence = VK_NULL_HANDLE;

    uint32_t numSignalSemaphores      = 0;
    vk::Semaphore signalSemaphores[2] = {};

    if (frameEnd)
    {
        if (pCmdLists && (m_PresentQueue != m_GpuQueues[pCmdLists->queueIndex]))
        {
            m_pendingPresentSemaphore             = m_frameFences[m_backBufferIndex].renderCompleteSemaphore;
            signalSemaphores[numSignalSemaphores] = m_pendingPresentSemaphore;
            ++numSignalSemaphores;
        }

        submitFence = m_frameFences[m_backBufferIndex].renderCompleteFence;
    }

    if (signalSemaphoreIndex != UINT32_MAX)
    {
        signalSemaphores[numSignalSemaphores] = m_queueSemaphores[signalSemaphoreIndex];
        ++numSignalSemaphores;
    }

    vk::SubmitInfo submitInfo       = {};
    submitInfo.commandBufferCount   = numCmdLists;
    submitInfo.pCommandBuffers      = pCmdBufs;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.waitSemaphoreCount   = numWaitSemaphores;
    submitInfo.pSignalSemaphores    = signalSemaphores;
    submitInfo.signalSemaphoreCount = numSignalSemaphores;
    submitInfo.pWaitDstStageMask    = &submitWaitStage;

    vk::Queue queue = pCmdLists ? m_GpuQueues[pCmdLists->queueIndex] : m_PresentQueue;
    VK_CHECK_RESULT(queue.submit(1, &submitInfo, submitFence));
}

void RenderSystem::EndCmdList(ActiveCommandList& cmdList)
{
    assert(cmdList.cmdBuf != VK_NULL_HANDLE);
    assert(cmdList.backBufferIndex == m_backBufferIndex);

    std::lock_guard<std::mutex> lock(m_cmdListMutex);

    cmdList.cmdBuf.end();

    m_cmdPools[cmdList.queueIndex][m_backBufferIndex][cmdList.poolIndex].inUse = false;
    cmdList.cmdPool                                                            = VK_NULL_HANDLE;
}

void RenderSystem::RecycleCmdList(ActiveCommandList& cmdList)
{
    cmdList.cmdBuf = VK_NULL_HANDLE;
}

void RenderSystem::ResetCommandPools()
{
    for (uint32_t iQ = 0; iQ < RPS_QUEUE_COUNT; iQ++)
    {
        if (m_backBufferIndex < m_cmdPools[iQ].size())
        {
            for (auto& pool : m_cmdPools[iQ][m_backBufferIndex])
            {
                if (!pool.cmdBuffers.empty())
                {
                    (*m_Device.Get()).freeCommandBuffers(pool.cmdPool, uint32_t(pool.cmdBuffers.size()), pool.cmdBuffers.data());
                    pool.cmdBuffers.clear();
                }
                (*m_Device.Get()).resetCommandPool(pool.cmdPool, {});
            }
        }
    }
}

void RenderSystem::ReserveSemaphores(uint32_t numSyncs)
{
    const uint32_t oldSize = uint32_t(m_queueSemaphores.size());
    if (numSyncs > oldSize)
    {
        m_queueSemaphores.resize(numSyncs, VK_NULL_HANDLE);
    }

    for (size_t i = oldSize; i < numSyncs; i++)
    {
        m_queueSemaphores[i] = (*m_Device.Get()).createSemaphore({});
    }
}

uint64_t RenderSystem::CalcGuaranteedCompletedFrameindexForRps() const
{
    // For VK we wait for swapchain before submitting, so max queued frame count is swapChainImages + 1.
    const uint32_t maxQueuedFrames = uint32_t(m_Swapchain.GetImageCount() + 1);

    return (m_FrameCounter > maxQueuedFrames) ? m_FrameCounter - maxQueuedFrames : RPS_GPU_COMPLETED_FRAME_INDEX_NONE;
}

// void RenderSystem::DrawTriangleWithRPSWrapper(const RpsCmdCallbackContext* pContext)
// {
//     vk::CommandBuffer cmd = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);
//     RenderSystem* renderSystem = reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);

//     renderSystem->DrawTriangle(cmd);
// }

// void RenderSystem::ShadowmapPassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
// {
//     RenderSystem* renderSystem = reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);

//     DrawKey key = { "ShadowCaster", AlphaMode::Opaque };

//     if (renderSystem->m_DrawData.find(key) != renderSystem->m_DrawData.end())
//     {
//         ScheduleDrawStream(*renderSystem->m_RenderContext, renderSystem->m_DrawData[key], rpsVKCommandBufferFromHandle(pContext->hCommandBuffer));
//     }
// }

// void RenderSystem::ForwardOpaquePassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
// {
//     RenderSystem* renderSystem = reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);

//     vk::CommandBuffer cmd = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);

//     VkImageView shadowmapView;
//     AssertIfRpsFailed(rpsVKGetCmdArgImageView(pContext, 0, &shadowmapView));
// }

// void RenderSystem::DrawTriangle(vk::CommandBuffer commandBuffer)
// {    
//     DrawKey key = { "ForwardPass", AlphaMode::Opaque };

//     if (m_DrawData.find(key) != m_DrawData.end())
//     {
//         ScheduleDrawStream(*m_RenderContext, m_DrawData[key], commandBuffer);
//     }

//     ScheduleDrawStream(*m_RenderContext, m_DrawData, commandBuffer);

//     ScheduleDraws(*m_RenderContext, m_DrawData, key, commandBuffer);

//     commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_RenderContext->GetGraphicsPipeline(m_TrianglePipelineHandle).pipeline);
//     commandBuffer.draw(3, 1, 0, 0);
// }

void RenderSystem::UploadPerframeGlobalConstantBuffer(uint32_t imageIndex)
{
    Camera* mainCamera = Camera::Main;
    if (mainCamera == nullptr)
    {
        return;
    }

    PerframeData perframeData;
    perframeData.vpMatrix = mainCamera->GetViewProjectionMatrix();
    
    // Update Main Light
    auto& gameObjects = Scene::GetActiveScene()->GetGameObjects();
    for (auto& gameObject : gameObjects)
    {
        Light* light = gameObject->GetComponent<Light>();
        if (light == nullptr)
            continue;
        
        Matrix4x4 lightMatrix = gameObject->GetTransform()->GetWorldToLocalMatrixIgnoreScale();
        Matrix4x4 orthoMatrix = Matrix4x4::CreateOrthographicLH(100.0f, 100.0f, .1f, 100.0f);
        perframeData.directionalLightVPMatrix = lightMatrix * orthoMatrix;
        
        LightData lightData = light->GetData();
        perframeData.directionalLightColor = lightData.color;
        perframeData.directionalLightIntensity = lightData.intensity;
        break;
    }

    m_RenderContext->CopyDataToBuffer(m_GlobalConstantBuffer, perframeData);
}

void RenderSystem::CreateDepthBuffer()
{
    std::vector<vk::Format> candidateFormats = {
        vk::Format::eD32Sfloat,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm
    };

    vk::Format depthFormat = vk::Format::eUndefined;

    const vk::raii::PhysicalDevice& physicalDevice = m_Device.GetPhysicalDevice().Get();

    for (auto& format : candidateFormats)
    {
        vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
        if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            depthFormat = format;
            break;
        }
    }

    bool hasStencil = depthFormat == vk::Format::eD32SfloatS8Uint ||
                      depthFormat == vk::Format::eD24UnormS8Uint ||
                      depthFormat == vk::Format::eD16UnormS8Uint;

    vk::Extent2D swapchainExtent = m_Swapchain.GetExtent();

    vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, depthFormat,
                                        vk::Extent3D(swapchainExtent.width, swapchainExtent.height, 1),
                                        1, 1,
                                        vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
                                        vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                        vk::SharingMode::eExclusive,
                                        {},
                                        vk::ImageLayout::eUndefined);

    // TODO: fill in all these fields
    VmaAllocationCreateInfo allocationCreateInfo{
        .flags          = 0,
        .usage          = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .requiredFlags  = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool           = VK_NULL_HANDLE,
        .pUserData      = nullptr,
        .priority       = 0
    };

    VkImage* depthImage = reinterpret_cast<VkImage*>(&m_DepthImage);
    VkImageCreateInfo* cImageCreateInfo = reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo);
    // TODO: allocate memory separately in the future
    VkResult res = vmaCreateImage(m_Device.GetVmaAllocator(), cImageCreateInfo, &allocationCreateInfo, depthImage, &m_DepthImageAllocation, nullptr);
    VK_CHECK_RESULT(res);

    m_Device.SetName(reinterpret_cast<uint64_t>(*depthImage), vk::ObjectType::eImage, "Depth Buffer");

    vk::ImageViewCreateInfo imageViewCreateInfo({},
                                                m_DepthImage,
                                                vk::ImageViewType::e2D,
                                                depthFormat,
                                                {},
                                                {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

    m_DepthImageView = m_Device.Get().createImageView(imageViewCreateInfo);

    m_Device.SetName(m_DepthImageView, "Depth Buffer ImageView");
}

void RenderSystem::CreateShadowPassObject()
{
    std::vector<Binding> bindings{
        {0, BindType::SampledImage, 1, ShaderStage::Fragment},
        {1, BindType::Sampler, 1, ShaderStage::Fragment}
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = 
    {
        .name = "Shadow Pass Descriptor Set Layout",
        .bindings = bindings
    };

    m_ShadowPassBindLayout = m_RenderContext->GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_ShadowmapSamplerHandler = m_RenderContext->CreateSampler({
        .debugName = "Shadowmap Sampler"
    });
}

void RenderSystem::CreateGlobalDescriptorSets()
{
    m_GlobalConstantBuffer = m_RenderContext->CreateBuffer({
        .debugName = "Global Constant Buffer",
        .byteSize  = sizeof(PerframeData),
        .usage     = BufferUsage::Uniform,
        .memUsage  = MemoryUsage::CPU_TO_GPU
    });

    std::vector<Binding> bindings {
        {0, BindType::UniformBuffer, 1, ShaderStage::Vertex | ShaderStage::Fragment}
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = 
    {
        .name = "Global Descriptor Set Layout",
        .bindings = bindings
    };

    m_GlobalBindLayout = m_RenderContext->GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_GlobalBindGroup = m_RenderContext->CreateBindGroup({
        .debugName = "Global BindGroup",
        .updateFrequency = UpdateFrequency::Persistent,
        .textures = {},
        .buffers = {{0, m_GlobalConstantBuffer, 0, sizeof(PerframeData), BindType::UniformBuffer}},
        .samplers = {},
        .bindLayout = &m_GlobalBindLayout,
    });
}

void RenderSystem::CreateMaterialDescriptorSets()
{
    std::vector<Binding> bindings {
        {0, BindType::SampledImage, 256, ShaderStage::Fragment},
        {1, BindType::Sampler, 1, ShaderStage::Fragment}
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = 
    {
        .name = "Bindless Material Descriptor Set Layout",
        .bindings = bindings
    };

    m_BindlessMaterialBinding.bindLayout = m_RenderContext->GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_BindlessMaterialBinding.albedoSampler = m_RenderContext->CreateSampler({
        .debugName = "Bindless Material Sampler"
    });

    m_BindlessMaterialBinding.bindGroup = m_RenderContext->CreateBindGroup({
        .debugName = "Bindless Material BindGroup",
        .updateFrequency = UpdateFrequency::Persistent,
        .textures = {
            {0, m_DefaultResources.whiteTexture, TextureUsageBits::Sampled, 0},
            {0, m_DefaultResources.blackTexture, TextureUsageBits::Sampled, 1}, 
            {0, m_DefaultResources.gridTexture, TextureUsageBits::Sampled, 2},
            {0, m_UVCheckTextureHandle, TextureUsageBits::Sampled, 3}},
        .buffers = {},
        .samplers = {{1, m_BindlessMaterialBinding.albedoSampler}},
        .bindLayout = &m_BindlessMaterialBinding.bindLayout,
    });
}

void RenderSystem:: CreateUVQuadDescriptorSets()
{
    std::vector<Binding> bindings {
        {0, BindType::CombinedSampledImage, 1, ShaderStage::Fragment}
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = 
    {
        .name = "UV Quad Descriptor Set Layout",
        .bindings = bindings
    };
    
    m_UVQuadBindLayout = m_RenderContext->GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_UVQuadBindGroup = m_RenderContext->CreateBindGroup({
        .debugName = "UV Quad BindGroup",
        .updateFrequency = UpdateFrequency::Persistent,
        .textures = { {0, m_UVCheckTextureHandle, TextureUsageBits::Sampled, 0, 0, BindType::CombinedSampledImage, m_UVCheckSamplerHandle}},
        .buffers = {},
        .samplers = {},
        .bindLayout = &m_UVQuadBindLayout,
    });
}

struct PerDrawData
{
    Matrix4x4 modelMatrix[128];
};

void RenderSystem::CreateDynamicUniformBuffer()
{
    size_t alignmentSize = utils::AlignUp(sizeof(PerDrawData), m_GraphicsCaps.minUniformBufferOffsetAlignment);

    size_t renderCount = 4;
    std::vector<uint8_t> dynamicUniformBufferData(alignmentSize * renderCount);
    
    for (size_t i = 0; i < renderCount; ++i)
    {
        PerDrawData* perDrawData = reinterpret_cast<PerDrawData*>(dynamicUniformBufferData.data() + (i * alignmentSize));
        perDrawData->modelMatrix[0] = Matrix4x4(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, i, 0.f, 0.f, 1.f);
    }

    m_DynamicUniformBuffer = m_RenderContext->CreateBuffer(
        {
            .debugName = "Dynamic Uniform Buffer",
            .byteSize = static_cast<uint32_t>(dynamicUniformBufferData.size()),
            .usage = BufferUsage::Uniform,
            .memUsage = MemoryUsage::GPU,
            .data = dynamicUniformBufferData.data()
        }
    );

    m_DynamicBufferHandle = m_RenderContext->CreateDynamicBuffer(
        {
            .debugName = "Dynamic Uniform Buffer",
            .buffer = m_DynamicUniformBuffer,
            .offset = 0,
            .range = sizeof(PerDrawData)    
        }
    );
}

static std::vector<char> LoadShaderBytecode(const std::string& name, const ShaderStage& stage, const std::string& entryPoint)
{
    static const std::filesystem::path kShaderSourceFolder = FileSystem::GetResourceFolder() / "Shaders";

    auto getShaderFile = [&name](ShaderStage stage) -> std::filesystem::path
    {
        std::filesystem::path path(name);
        auto shaderPath = kShaderSourceFolder / path.parent_path() / path.filename().stem();
        shaderPath += std::string(".") + (stage == ShaderStage::Vertex ? "vert" : "frag") + ".spv";
        return shaderPath;
    };

    std::filesystem::path shaderPath = getShaderFile(stage);

    return FileSystem::ReadAllBinary(shaderPath);
}

void RenderSystem::CreatePipeline()
{
    RenderPass renderPass = m_RenderContext->CreateRenderPass(
        RenderPassDesc
        {
            {GraphicsFormat::BGRA8_SRGB}
        }
    );

    std::vector<char> cubeVertBytecode = LoadShaderBytecode("sample/cube", ShaderStage::Vertex, "vs");
    std::vector<char> cubeFragBytecode = LoadShaderBytecode("sample/cube", ShaderStage::Fragment, "ps");

    m_CubePipelineHandle = m_RenderContext->CreateGraphicsPipeline(
        GraphicsPipelineDesc
        {
            .debugName = "Cube Pipeline",
            .VS
            {
                .byteCode = reinterpret_cast<uint8_t*>(cubeVertBytecode.data()),
                .byteSize = static_cast<uint32_t>(cubeVertBytecode.size()), 
                .entryFunc = "vs"
            },
            .PS
            {
                .byteCode = reinterpret_cast<uint8_t*>(cubeFragBytecode.data()), 
                .byteSize = static_cast<uint32_t>(cubeFragBytecode.size()), 
                .entryFunc = "ps"
            },
            .colorFormats = { GraphicsFormat::BGRA8_SRGB },
            .depthFormat = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .bindLayouts = { m_GlobalBindLayout },
            .dynamicBuffer = m_DynamicBufferHandle,
            .renderPass = renderPass.renderPass,
            .subpassIndex = 0
        }
    );

    // std::vector<char> unlitVertBytecode = LoadShaderBytecode("sample/UnLit", ShaderStage::Vertex, "vs");
    // std::vector<char> unlitFragBytecode = LoadShaderBytecode("sample/UnLit", ShaderStage::Fragment, "ps");

    std::vector<char> triangleVertBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Vertex, "vs");
    std::vector<char> triangleFragBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Fragment, "ps");

    // std::vector<char> quadVertBytecode = LoadShaderBytecode("sample/quad", ShaderStage::Vertex, "vs");
    // std::vector<char> quadFragBytecode = LoadShaderBytecode("sample/quad", ShaderStage::Fragment, "ps");

    // m_UnLitPipelineHandle = m_RenderContext->CreateGraphicsPipeline(
    //     GraphicsPipelineDesc
    //     {
    //         .debugName = "UnLit Pipeline",
    //         .VS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(unlitVertBytecode.data()),
    //             .byteSize = static_cast<uint32_t>(unlitVertBytecode.size()), 
    //             .entryFunc = "vs"
    //         },
    //         .PS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(unlitFragBytecode.data()), 
    //             .byteSize = static_cast<uint32_t>(unlitFragBytecode.size()), 
    //             .entryFunc = "ps"
    //         },
    //         .colorFormats = { GraphicsFormat::BGRA8_SRGB },
    //         .depthFormat = GraphicsFormat::D32_FLOAT,
    //         .stencilFormat = GraphicsFormat::Undefined,
    //         .vertexBufferBindings
    //         {
    //             {
    //                 .byteStride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3), 
    //                 .attributes = 
    //                 {
    //                     { .byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT },
    //                     { .byteOffset = 12, .format = GraphicsFormat::RG32_FLOAT },
    //                     { .byteOffset = 20, .format = GraphicsFormat::RGB32_FLOAT }
    //                 }
    //             }
    //         },
    //         .bindLayouts {},
    //         .subpassIndex = 0
    //     }
    // );

    m_TrianglePipelineHandle = m_RenderContext->CreateGraphicsPipeline(
        GraphicsPipelineDesc
        {
            .debugName = "Triangle Pipeline",
            .VS
            {
                .byteCode = reinterpret_cast<uint8_t*>(triangleVertBytecode.data()),
                .byteSize = static_cast<uint32_t>(triangleVertBytecode.size()), 
                .entryFunc = "vs"
            },
            .PS
            {
                .byteCode = reinterpret_cast<uint8_t*>(triangleFragBytecode.data()), 
                .byteSize = static_cast<uint32_t>(triangleFragBytecode.size()), 
                .entryFunc = "ps"
            },            
            .colorFormats = {GraphicsFormat::BGRA8_SRGB},
            .depthFormat = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .bindLayouts {},
            .renderPass = renderPass.renderPass,
            .subpassIndex = 0
        }
    );

    // m_QuadPipelineHandle = m_RenderContext->CreateGraphicsPipeline(
    //     GraphicsPipelineDesc
    //     {
    //         .debugName = "UV Quad Pipeline",
    //         .VS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(quadVertBytecode.data()),
    //             .byteSize = static_cast<uint32_t>(quadVertBytecode.size()), 
    //             .entryFunc = "vs"
    //         },
    //         .PS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(quadFragBytecode.data()), 
    //             .byteSize = static_cast<uint32_t>(quadFragBytecode.size()), 
    //             .entryFunc = "ps"
    //         },
    //         .colorFormats = {GraphicsFormat::BGRA8_SRGB},
    //         .depthFormat = GraphicsFormat::D32_FLOAT,
    //         .stencilFormat = GraphicsFormat::Undefined,
    //         .bindLayouts { m_UVQuadBindLayout },
    //         .subpassIndex = 0
    //     }
    // );

    m_RenderContext->DestroyRenderPass(renderPass);
}

void RenderSystem::CreateTextureObjects()
{
    m_UVCheckTextureHandle = m_RenderContext->CreateTextureHandle("sample.jpg");

    m_UVCheckSamplerHandle = m_RenderContext->CreateSampler(
        SamplerDesc
        {
            .debugName = "UV Check Sampler",
        }
    );
}

void RenderSystem::GetQueues()
{
    struct QueueFamilyIndexSelection
    {
        uint32_t first    = UINT32_MAX;
        uint32_t prefered = UINT32_MAX;

        uint32_t Get() const
        {
            return (prefered != UINT32_MAX) ? prefered : first;
        }
    };

    void* nativeWindowHandle = m_App->GetWindow()->GetNativeHandle();

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_Device.GetQueueFamilyProperties();

    QueueFamilyIndexSelection presentQueueSel;
    QueueFamilyIndexSelection graphicsQueueSel;
    QueueFamilyIndexSelection computeQueueSel;
    QueueFamilyIndexSelection copyQueueSel;

    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        bool supportsPresent = m_Device.GetPhysicalDevice().QueueFamilyIsPresentable(i, nativeWindowHandle);

        bool hasGfx     = static_cast<bool>(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics);
        bool hasCompute = static_cast<bool>(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute);
        bool hasCopy    = static_cast<bool>(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer);

        if (supportsPresent)
        {
            if (presentQueueSel.first == UINT32_MAX)
                presentQueueSel.first = i;

            if (hasGfx)
                presentQueueSel.prefered = i;
        }

        if (hasGfx)
        {
            if (graphicsQueueSel.first == UINT32_MAX)
                graphicsQueueSel.first = i;

            if (supportsPresent)
                graphicsQueueSel.prefered = i;
        }

        if (hasCompute)
        {
            if (computeQueueSel.first == UINT32_MAX)
                computeQueueSel.first = i;

            if (!hasGfx)
                computeQueueSel.prefered = i;
        }

        if (hasCopy)
        {
            if (copyQueueSel.first == UINT32_MAX)
                copyQueueSel.first = i;

            if (!hasCompute)
                copyQueueSel.prefered = i;
        }
    }

    m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS] = graphicsQueueSel.Get();
    m_GpuQueueFamilyIndices[RPS_QUEUE_COMPUTE]  = computeQueueSel.Get();
    m_GpuQueueFamilyIndices[RPS_QUEUE_COPY]     = copyQueueSel.Get();

    m_PresentQueueFamilyIndex = presentQueueSel.Get();

    m_GpuQueues[RPS_QUEUE_GRAPHICS] = (*m_Device.Get()).getQueue(m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], 0);
    m_Device.SetName(*reinterpret_cast<uint64_t*>(&m_GpuQueues[RPS_QUEUE_GRAPHICS]), vk::ObjectType::eQueue, "Graphics Queue");

    m_GpuQueues[RPS_QUEUE_COMPUTE] = (*m_Device.Get()).getQueue(m_GpuQueueFamilyIndices[RPS_QUEUE_COMPUTE], 0);
    m_Device.SetName(*reinterpret_cast<uint64_t*>(&m_GpuQueues[RPS_QUEUE_COMPUTE]), vk::ObjectType::eQueue, "Compute Queue");
    
    m_GpuQueues[RPS_QUEUE_COPY] = (*m_Device.Get()).getQueue(m_GpuQueueFamilyIndices[RPS_QUEUE_COPY], 0);
    m_Device.SetName(*reinterpret_cast<uint64_t*>(&m_GpuQueues[RPS_QUEUE_COPY]), vk::ObjectType::eQueue, "Copy Queue");

    m_PresentQueue  = (*m_Device.Get()).getQueue(m_PresentQueueFamilyIndex, 0);
    m_Device.SetName(*reinterpret_cast<uint64_t*>(&m_PresentQueue), vk::ObjectType::eQueue, "Present Queue");
}

const PhysicalDevice& RenderSystem::GetBestDevice(const std::vector<PhysicalDevice>& devices) const
{
    int maxScore = -1;
    int physicalDeviceIndex = -1;
    for (int i = 0; i < devices.size(); ++i)
    {
        devices[i].Output();

        int score = devices[i].Score();
        if (score >= maxScore)
        {
            maxScore            = score;
            physicalDeviceIndex = i;
        }
    }

    if (physicalDeviceIndex < 0)
    {
        LOG_STREAM(FATAL) << "No suitable physical device found" << std::endl;
        return devices[0];
    }

    return devices[physicalDeviceIndex];
}

void RenderSystem::CreateRpsPipelines()
{
    // Forward Pipeline
    RenderPassDesc forwardPassDesc = {{GraphicsFormat::BGRA8_SRGB}};
    AutoRenderPass forwardPass(m_RenderContext.get(), forwardPassDesc);

    std::vector<char> vertexShaderByteCode = LoadShaderBytecode("sample/SimpleLit", ShaderStage::Vertex, "main");
    std::vector<char> fragmentShaderByteCode = LoadShaderBytecode("sample/SimpleLit", ShaderStage::Fragment, "main");

    m_RpsPipelines.forwardPipeline = m_RenderContext->CreateGraphicsPipeline(
        GraphicsPipelineDesc{
            .debugName = "SimpleLit",
            .VS{
                .byteCode  = reinterpret_cast<uint8_t*>(vertexShaderByteCode.data()),
                .byteSize  = static_cast<uint32_t>(vertexShaderByteCode.size()),
                .entryFunc = "vs"},
            .PS{
                .byteCode  = reinterpret_cast<uint8_t*>(fragmentShaderByteCode.data()),
                .byteSize  = static_cast<uint32_t>(fragmentShaderByteCode.size()),
                .entryFunc = "ps"},
            .colorFormats  = {GraphicsFormat::BGRA8_SRGB},
            .depthFormat   = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .vertexBufferBindings{
                {.byteStride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3),
                 .attributes =
                     {
                         {.byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT},
                         {.byteOffset = 12, .format = GraphicsFormat::RG32_FLOAT},
                         {.byteOffset = 20, .format = GraphicsFormat::RGB32_FLOAT}}}},
            .bindLayouts   = { m_GlobalBindLayout, m_ShadowPassBindLayout, m_BindlessMaterialBinding.bindLayout },
            .dynamicBuffer = m_DynamicBufferHandle,
            .renderPass    = forwardPass.GetRenderPass().renderPass,
            .subpassIndex  = 0
    });

    // Shadow Pipeline
    RenderPassDesc shadowPassDesc = {{}, GraphicsFormat::D32_FLOAT};
    AutoRenderPass shadowPass(m_RenderContext.get(), shadowPassDesc);

    std::vector<char> vertexShaderBytecode   = LoadShaderBytecode("sample/Shadowmap", ShaderStage::Vertex, "main");
    std::vector<char> fragmentShaderBytecode = LoadShaderBytecode("sample/Shadowmap", ShaderStage::Fragment, "main");

    m_RpsPipelines.shadowPipeline = m_RenderContext->CreateGraphicsPipeline({
        GraphicsPipelineDesc{
            .debugName = "Shadowmap Pipeline",
            .VS{
                .byteCode  = reinterpret_cast<uint8_t*>(vertexShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(vertexShaderBytecode.size()),
                .entryFunc = "vs"},
            .PS{
                .byteCode  = reinterpret_cast<uint8_t*>(fragmentShaderBytecode.data()),
                .byteSize  = static_cast<uint32_t>(fragmentShaderBytecode.size()),
                .entryFunc = "ps"},
            .colorFormats  = {},
            .depthFormat   = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .vertexBufferBindings{
                {.byteStride = sizeof(Vector3) + sizeof(Vector2) + sizeof(Vector3),
                 .attributes =
                     {
                         {.byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT},
                         {.byteOffset = 12, .format = GraphicsFormat::RG32_FLOAT},
                         {.byteOffset = 20, .format = GraphicsFormat::RGB32_FLOAT}}}},
            .bindLayouts   = { m_GlobalBindLayout },
            .dynamicBuffer = m_DynamicBufferHandle,
            .renderPass    = shadowPass.GetRenderPass().renderPass,
            .subpassIndex  = 0
        }
    });

    Material& forwardMat = m_RpsMaterial.forward;
    forwardMat.SetAlphaMode(AlphaMode::Opaque);
    forwardMat.SetDynamicBuffer(m_DynamicBufferHandle);
    forwardMat.AddPass(Pass{
        .name = "ShadowCaster",
        .shader = m_RpsPipelines.shadowPipeline,
        .bindGroup = {m_GlobalBindGroup},
    });

    Pass forwardOpaquePass;
    forwardOpaquePass.name = "ForwardPass";
    forwardOpaquePass.shader = m_RpsPipelines.forwardPipeline;
    forwardOpaquePass.bindGroup[0] = m_GlobalBindGroup;
    forwardOpaquePass.bindGroup[2] = m_BindlessMaterialBinding.bindGroup;

    forwardMat.AddPass(forwardOpaquePass);
}

void RenderSystem::CreateDefaultResources()
{
    std::vector<uint8_t> blackTextureData(4, 0);
    std::vector<uint8_t> whiteTextureData(4, 255);

    m_DefaultResources.blackTexture = m_RenderContext->CreateTextureHandle(
        TextureDesc
        {
            .debugName = "Black Texture",
            .width     = 1,
            .height    = 1,
            .data      = blackTextureData.data(),
            .dataSize  = 4,
        });

    m_DefaultResources.whiteTexture = m_RenderContext->CreateTextureHandle(
        TextureDesc
        {
            .debugName = "White Texture",
            .width     = 1,
            .height    = 1,
            .data      = whiteTextureData.data(),
            .dataSize  = 4,
        });

    m_DefaultResources.gridTexture = m_RenderContext->CreateTextureHandle("grid.jpg");
}

void RenderSystem::UpdateGlobalConstantBuffer()
{
    Camera* mainCamera = Camera::Main;
    if (mainCamera == nullptr)
    {
        return;
    }

    PerframeData perframeData;
    perframeData.vpMatrix = mainCamera->GetViewProjectionMatrix();
    
    // Update Main Light
    auto& gameObjects = Scene::GetActiveScene()->GetGameObjects();
    for (auto& gameObject : gameObjects)
    {
        Light* light = gameObject->GetComponent<Light>();
        if (light == nullptr)
            continue;
        
        Matrix4x4 lightMatrix = gameObject->GetTransform()->GetWorldToLocalMatrixIgnoreScale();
        Matrix4x4 orthoMatrix = Matrix4x4::CreateOrthographicLH(100.0f, 100.0f, .1f, 100.0f);
        perframeData.directionalLightVPMatrix = lightMatrix * orthoMatrix;
        
        LightData lightData = light->GetData();
        perframeData.directionalLightColor = lightData.color;
        perframeData.directionalLightIntensity = lightData.intensity;
        break;
    }

    m_RenderContext->CopyDataToBuffer(m_GlobalConstantBuffer, perframeData);
}

void RenderSystem::ShadowmapPassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    RenderSystem& renderSystem = *reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);
    vk::CommandBuffer cmd      = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);
    
    DrawKey key = {"ShadowCaster", AlphaMode::Opaque};
    
    renderSystem.DrawRenderer(key, cmd, renderSystem.m_RpsPipelines.shadowPipeline);
}

void RenderSystem::ForwardOpaquePassWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    RenderSystem& renderSystem = *reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);
    vk::CommandBuffer cmd      = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);
    
    // Update ShadowMap Descriptor Set
    VkImageView shadowmapView;
    RpsResult result = rpsVKGetCmdArgImageView(pContext, 0, &shadowmapView);
    if (RPS_SUCCEEDED(result) == true)
    {
        auto& renderContext = renderSystem.m_RenderContext;
        
        auto shadowmapBindGroup = renderContext->CreateTransientBindGroup({
            .debugName = "Shadowmap BindGroup",
            .updateFrequency = UpdateFrequency::PerFrame,
            .bindLayout = &renderSystem.m_ShadowPassBindLayout,
        });

        renderContext->UpdateBindGroup(shadowmapBindGroup, 
            {
                .samplers = {{1, renderSystem.m_ShadowmapSamplerHandler}},
            },
            {
                .textures = {{0, shadowmapView}},
            }
        );

        auto& pipeline = renderContext->GetGraphicsPipeline(renderSystem.m_RpsPipelines.forwardPipeline);

        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.layout, 1, {shadowmapBindGroup.descriptorSet}, {});
    }

    DrawKey key = {"ForwardPass", AlphaMode::Opaque};

    renderSystem.DrawRenderer(key, cmd, renderSystem.m_RpsPipelines.forwardPipeline);
}

void RenderSystem::ResetPerFrameDescriptorPool()
{
    m_RenderContext->ResetDescriptorPool(UpdateFrequency::PerFrame);
}
} // namespace gore
