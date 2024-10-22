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

#include "Rendering/GPUData/GlobalConstantBuffer.h"
#include "RenderContextHelper.h"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>

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

    InitVulkanGraphicsCaps(m_GraphicsCaps, *m_Instance.Get(), *m_Device.GetPhysicalDevice().Get());

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
    
    CreateDepthBuffer();
    
    CreateTextureObjects();

    CreateGlobalDescriptorSets();
    CreateUVQuadDescriptorSets();
    CreateDynamicUniformBuffer();
    CreatePipeline();
    GetQueues();

    InitImgui();
}

void RenderSystem::PrepareDrawData()
{
    DrawCreateInfo info = {};
    info.passName = "ForwardPass";
    info.alphaMode = AlphaMode::Opaque;

    std::vector<GameObject*> gameObjects = Scene::GetActiveScene()->GetGameObjects();
    std::vector<Draw> sortedDrawData;
    PrepareDrawDataAndSort(info, gameObjects, sortedDrawData);

    // TODO: check if the draw data is already in the map
    m_DrawData.clear();

    DrawCacheKey key = {};
    key.passName = info.passName;
    key.alphaMode = info.alphaMode;

    m_DrawData[key] = sortedDrawData;
}

void RenderSystem::Update()
{
    PrepareDrawData();

    return RunRpsSystem();

    // ImGui_ImplVulkan_NewFrame();
    // ImGui_ImplGlfw_NewFrame();
    // ImGui::NewFrame();

    // bool show = true;
    // ImGui::ShowDemoWindow(&show);

    // ImGui::Render();

    // Window* window = m_App->GetWindow();

    // Camera* camera = nullptr;
    // for (auto& gameObject : Scene::GetActiveScene()->GetGameObjects())
    // {
    //     camera = gameObject->GetComponent<Camera>();
    //     if (camera != nullptr)
    //         break;
    // }

    // uint32_t currentSwapchainImageIndex = m_Swapchain.GetCurrentImageIndex();
    // vk::Extent2D surfaceExtent = m_Swapchain.GetExtent();
    // const std::vector<vk::Image>& swapchainImages = m_Swapchain.GetImages();
    // const std::vector<vk::raii::ImageView>& swapchainImageViews = m_Swapchain.GetImageViews();

    // CommandRingElement commandElement = RequestNextCommandElement(m_GraphicsCommandRing.get(), true, 1);

    // vk::Fence inFlightFence = commandElement.fence->fence;

    // vk::Result result = m_Device.Get().waitForFences({inFlightFence}, true, UINT64_MAX);
    // m_Device.Get().resetFences({inFlightFence});

    // m_RenderContext->ResetCommandPool(commandElement.cmdPool);

    // vk::CommandBuffer commandBuffer = commandElement.cmdBuffer[0]->cmdBuffer;

    // vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    // commandBuffer.begin(beginInfo);

    // std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers;
    // imageMemoryBarriers.emplace_back(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eColorAttachmentWrite,
    //                                  vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
    //                                  m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], swapchainImages[currentSwapchainImageIndex],
    //                                  vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    // commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {}, imageMemoryBarriers);

    // std::vector<vk::ImageMemoryBarrier> depthImageMemoryBarriers;
    // depthImageMemoryBarriers.emplace_back(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    //                                       vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
    //                                       m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS],
    //                                       m_DepthImage,
    //                                       vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
    // commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eEarlyFragmentTests, {}, {}, {}, depthImageMemoryBarriers);

    // vk::ClearValue clearValueColor(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
    // vk::ClearValue clearValueDepth(vk::ClearDepthStencilValue(0.0f, 0));
    // std::vector<vk::ClearValue> clearValues = {clearValueColor, clearValueDepth};
    
    // vk::RenderingAttachmentInfoKHR renderingAttachmentInfo(*swapchainImageViews[currentSwapchainImageIndex], vk::ImageLayout::eColorAttachmentOptimal);
    // renderingAttachmentInfo.setClearValue(clearValueColor); 
    // renderingAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
    // renderingAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);

    // vk::RenderingAttachmentInfoKHR depthStencilAttachmentInfo(*m_DepthImageView, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    // depthStencilAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
    // depthStencilAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eDontCare);

    // vk::RenderingInfoKHR renderingInfo({}, vk::Rect2D{{0, 0}, surfaceExtent}, 1, 0, 1, &renderingAttachmentInfo, &depthStencilAttachmentInfo);

    // commandBuffer.beginRenderingKHR(renderingInfo);

    // vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(surfaceExtent.width), static_cast<float>(surfaceExtent.height), 0.0f, 1.0f);
    // commandBuffer.setViewport(0, {viewport});

    // vk::Rect2D scissor({0, 0}, surfaceExtent);
    // commandBuffer.setScissor(0, {scissor});

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_RenderContext->GetGraphicsPipeline(m_TrianglePipelineHandle).pipeline);
    // commandBuffer.draw(3, 1, 0, 0);

    // auto& quadPipeline = m_RenderContext->GetGraphicsPipeline(m_QuadPipelineHandle);

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, quadPipeline.pipeline);

    // vk::DescriptorSet descriptor = m_RenderContext->GetBindGroup(m_UVQuadBindGroup).set;

    // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, quadPipeline.layout, 0, { descriptor }, {});
    // commandBuffer.draw(6, 1, 0, 0);

    

    // auto& globalConstantBuffer = m_RenderContext->GetBuffer(m_GlobalConstantBuffer);

    // void* mappedData;
    // vmaMapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vmaAllocation, &mappedData);
    // auto& globalConstantBufferData = *reinterpret_cast<GlobalConstantBuffer*>(mappedData);
    // globalConstantBufferData.vpMatrix = camera->GetViewProjectionMatrix();
    // vmaUnmapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vmaAllocation);

    // auto& unlitPipeline = m_RenderContext->GetGraphicsPipeline(m_UnLitPipelineHandle);

    // uint32_t dynamicAlignment = 64;

    // auto& cubePipeline = m_RenderContext->GetGraphicsPipeline(m_CubePipelineHandle);

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubePipeline.pipeline);
    // auto& globalBindGroup = m_RenderContext->GetBindGroup(m_GlobalBindGroup);
    // auto& dynamicBuffer = m_RenderContext->GetDynamicBuffer(m_DynamicBufferHandle);
    // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubePipeline.layout, 0, {globalBindGroup.set}, {});
    // for (int i = 0; i < 4; i++)
    // {
    //     commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubePipeline.layout, 3, { dynamicBuffer.set}, { i * dynamicAlignment });
    //     commandBuffer.draw(36, 1, 0, 0);
    // }

    // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, unlitPipeline.layout, 0, {*m_GlobalDescriptorSets[currentSwapchainImageIndex]}, {});

    // for (auto& gameObject : Scene::GetActiveScene()->GetGameObjects())
    // {
    //     if (gameObject == camera->GetGameObject())
    //         continue;

    //     MeshRenderer* meshRenderer = gameObject->GetComponent<MeshRenderer>();
    //     if (meshRenderer == nullptr)
    //         continue;

    //     auto& vertexBuffer = m_RenderContext->GetBuffer(meshRenderer->GetVertexBuffer());
    //     commandBuffer.bindVertexBuffers(0, {vertexBuffer.vkBuffer}, {0});

    //     IndexType indexType = meshRenderer->GetIndexType();
    //     if (indexType != IndexType::None)
    //     {
    //         auto& indexBuffer = m_RenderContext->GetBuffer(meshRenderer->GetIndexBuffer());
    //         commandBuffer.bindIndexBuffer(indexBuffer.vkBuffer, 0, VulkanHelper::GetVkIndexType(indexType));
    //     }

    //     commandBuffer.drawIndexed(meshRenderer->GetIndexCount(), 1, 0, 0, 0);
    // }

    // commandBuffer.endRenderingKHR();

    // renderingAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eLoad);
    // vk::RenderingInfoKHR imguiRenderInfo({}, vk::Rect2D{{0, 0}, surfaceExtent}, 1, 0, 1, &renderingAttachmentInfo, nullptr);
    // commandBuffer.beginRenderingKHR(imguiRenderInfo);

    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    // commandBuffer.endRenderingKHR();

    // std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers2;
    // imageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
    //                                   vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
    //                                   m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], swapchainImages[currentSwapchainImageIndex],
    //                                   vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    // commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, imageMemoryBarriers2);

    // std::vector<vk::ImageMemoryBarrier> depthImageMemoryBarriers2;
    // depthImageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
    //                                        vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
    //                                        m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS], m_GpuQueueFamilyIndices[RPS_QUEUE_GRAPHICS],
    //                                        m_DepthImage,
    //                                        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
    // commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, depthImageMemoryBarriers2);

    // commandBuffer.end();

    // std::vector<vk::Semaphore> waitSemaphores = {};
    // std::vector<vk::PipelineStageFlags> waitStages = {};
    // std::vector<vk::CommandBuffer> submitCommandBuffers = { commandBuffer };
    // std::vector<vk::Semaphore> renderFinishedSemaphores = { commandElement.semaphore->semaphore };
    // vk::SubmitInfo submitInfo(waitSemaphores, waitStages, submitCommandBuffers, renderFinishedSemaphores);
    // m_GpuQueues[RPS_QUEUE_GRAPHICS].submit({submitInfo}, inFlightFence);

    // bool recreated = m_Swapchain.Present(renderFinishedSemaphores, m_PresentQueue);

    // if (recreated)
    // {
    //     if (m_DepthImage != nullptr)
    //     {
    //         m_DepthImageView = nullptr;
    //         vmaDestroyImage(m_Device.GetVmaAllocator(), m_DepthImage, m_DepthImageAllocation);
    //     }
    //     int width, height;
    //     window->GetSize(&width, &height);
    //     CreateDepthBuffer();

    //     m_Device.SetName(m_Swapchain.Get(), "Main Swapchain");
    // }
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
    init_info.UseDynamicRendering = true;
    init_info.ColorAttachmentFormat = static_cast<VkFormat>(m_Swapchain.GetFormat().format);
	ImGui_ImplVulkan_Init(&init_info, nullptr);
}

void RenderSystem::ShutdownImgui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_ImguiDescriptorPool.clear();
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
    AssertIfRpsFailed(rpsProgramBindNode(rpsRenderGraphGetMainEntry(renderGraph), "Triangle", &DrawTriangleWithRPSWrapper, this));
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

    ResetCommandPools();

    ExecuteRenderGraph(m_FrameCounter, *m_RpsSystem->rpsRDG);

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

void RenderSystem::DrawTriangleWithRPSWrapper(const RpsCmdCallbackContext* pContext)
{
    vk::CommandBuffer cmd = rpsVKCommandBufferFromHandle(pContext->hCommandBuffer);
    RenderSystem* renderSystem = reinterpret_cast<RenderSystem*>(pContext->pUserRecordContext);

    renderSystem->DrawTriangle(cmd);
}

void RenderSystem::DrawTriangle(vk::CommandBuffer commandBuffer)
{    
    DrawCacheKey key = { "ForwardPass", AlphaMode::Opaque };

    ScheduleDraws(*m_RenderContext, m_DrawData, key, commandBuffer);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_RenderContext->GetGraphicsPipeline(m_TrianglePipelineHandle).pipeline);
    commandBuffer.draw(3, 1, 0, 0);
}

void RenderSystem::UploadPerframeGlobalConstantBuffer(uint32_t imageIndex)
{
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

void RenderSystem::CreateGlobalDescriptorSets()
{
    m_GlobalConstantBuffer = m_RenderContext->CreateBuffer({
        .debugName = "Global Constant Buffer",
        .byteSize  = sizeof(GlobalConstantBuffer),
        .usage     = BufferUsage::Uniform,
        .memUsage  = MemoryUsage::CPU_TO_GPU
    });

    std::vector<Binding> bindings {
        {0, BindType::UniformBuffer, 1, ShaderStage::Vertex}
    };

    BindLayoutCreateInfo bindLayoutCreateInfo = 
    {
        .name = "Global Descriptor Set Layout",
        .bindings = bindings
    };

    m_GlobalBindLayout = m_RenderContext->GetOrCreateBindLayout(bindLayoutCreateInfo);

    m_GlobalBindGroup = m_RenderContext->CreateBindGroup({
        .debugName = "Global BindGroup",
        .updateFrequency = UpdateFrequency::PerFrame,
        .textures = {},
        .buffers = {{0, m_GlobalConstantBuffer, 0, sizeof(GlobalConstantBuffer), BindType::UniformBuffer}},
        .samplers = {},
        .bindLayout = &m_GlobalBindLayout,
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
        .updateFrequency = UpdateFrequency::PerFrame,
        .textures = { {0, m_UVCheckTextureHandle, TextureUsageBits::Sampled, 0, 0, BindType::CombinedSampledImage, m_UVCheckSamplerHandle}},
        .buffers = {},
        .samplers = {},
        .bindLayout = &m_UVQuadBindLayout,
    });
}

struct PerDrawData
{
    Matrix4x4 modelMatrix;
};

void RenderSystem::CreateDynamicUniformBuffer()
{
    size_t alignmentSize = utils::AlignUp(sizeof(PerDrawData), m_GraphicsCaps.minUniformBufferOffsetAlignment);

    size_t renderCount = 4;
    std::vector<uint8_t> dynamicUniformBufferData(alignmentSize * renderCount);
    
    for (size_t i = 0; i < renderCount; ++i)
    {
        PerDrawData* perDrawData = reinterpret_cast<PerDrawData*>(dynamicUniformBufferData.data() + (i * alignmentSize));
        perDrawData->modelMatrix = Matrix4x4(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, i, 0.f, 0.f, 1.f);
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
} // namespace gore