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
    m_GraphicsQueue(nullptr),
    m_GraphicsQueueFamilyIndex(0),
    m_PresentQueue(nullptr),
    m_PresentQueueFamilyIndex(0),
    // Command Pool & Command Buffer
    m_GraphicsCommandRing(),
    // Depth Buffer
    m_DepthImage(nullptr),
    m_DepthImageAllocation(VK_NULL_HANDLE),
    m_DepthImageView(nullptr),
    // Imgui
    m_ImguiWindowData(),
    m_ImguiDescriptorPool(nullptr),
    // Utils
    m_RenderDeletionQueue()
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

	VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Device.Get());

    m_Swapchain = m_Device.CreateSwapchain(window->GetNativeHandle(), 3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    m_Device.SetName(m_Swapchain.Get(), "Main Swapchain");

    InitVulkanGraphicsCaps(m_GraphicsCaps, *m_Instance.Get(), *m_Device.GetPhysicalDevice().Get());

    m_RenderContext = std::make_unique<RenderContext>(&m_Device);
    m_RenderContext->PrepareRendering();

    CommandRingCreateDesc cmdRingDesc = {};
    cmdRingDesc.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    cmdRingDesc.cmdPoolCount = 3;
    cmdRingDesc.cmdBufferCountPerPool = 1;
    cmdRingDesc.addSyncObjects = true;
#if ENGINE_DEBUG
    cmdRingDesc.debugName = "Graphics Command Ring";
#endif

    m_GraphicsCommandRing = m_RenderContext->CreateCommandRing(cmdRingDesc);

    m_RenderDeletionQueue.PushFunction([this]()
    {
        m_RenderContext->DestroyCommandRing(m_GraphicsCommandRing);
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

void RenderSystem::Update()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    bool show = true;
    ImGui::ShowDemoWindow(&show);

    ImGui::Render();

    Window* window = m_App->GetWindow();

    Camera* camera = nullptr;
    for (auto& gameObject : Scene::GetActiveScene()->GetGameObjects())
    {
        camera = gameObject->GetComponent<Camera>();
        if (camera != nullptr)
            break;
    }

    uint32_t currentSwapchainImageIndex = m_Swapchain.GetCurrentImageIndex();
    vk::Extent2D surfaceExtent = m_Swapchain.GetExtent();
    const std::vector<vk::Image>& swapchainImages = m_Swapchain.GetImages();
    const std::vector<vk::raii::ImageView>& swapchainImageViews = m_Swapchain.GetImageViews();

    CommandRingElement commandElement = RequestNextCommandElement(m_GraphicsCommandRing.get(), true, 1);

    vk::Fence inFlightFence = commandElement.fence->fence;

    vk::Result result = m_Device.Get().waitForFences({inFlightFence}, true, UINT64_MAX);
    m_Device.Get().resetFences({inFlightFence});

    m_RenderContext->ResetCommandPool(commandElement.cmdPool);

    vk::CommandBuffer commandBuffer = commandElement.cmdBuffer[0]->cmdBuffer;

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers;
    imageMemoryBarriers.emplace_back(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eColorAttachmentWrite,
                                     vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                                     m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex, swapchainImages[currentSwapchainImageIndex],
                                     vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {}, imageMemoryBarriers);

    std::vector<vk::ImageMemoryBarrier> depthImageMemoryBarriers;
    depthImageMemoryBarriers.emplace_back(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                                          vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                          m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex,
                                          m_DepthImage,
                                          vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eEarlyFragmentTests, {}, {}, {}, depthImageMemoryBarriers);

    vk::ClearValue clearValueColor(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
    vk::ClearValue clearValueDepth(vk::ClearDepthStencilValue(0.0f, 0));
    std::vector<vk::ClearValue> clearValues = {clearValueColor, clearValueDepth};
    
    vk::RenderingAttachmentInfoKHR renderingAttachmentInfo(*swapchainImageViews[currentSwapchainImageIndex], vk::ImageLayout::eColorAttachmentOptimal);
    renderingAttachmentInfo.setClearValue(clearValueColor); 
    renderingAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
    renderingAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);

    vk::RenderingAttachmentInfoKHR depthStencilAttachmentInfo(*m_DepthImageView, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    depthStencilAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
    depthStencilAttachmentInfo.setStoreOp(vk::AttachmentStoreOp::eDontCare);

    vk::RenderingInfoKHR renderingInfo({}, vk::Rect2D{{0, 0}, surfaceExtent}, 1, 0, 1, &renderingAttachmentInfo, &depthStencilAttachmentInfo);

    commandBuffer.beginRenderingKHR(renderingInfo);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(surfaceExtent.width), static_cast<float>(surfaceExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, {viewport});

    vk::Rect2D scissor({0, 0}, surfaceExtent);
    commandBuffer.setScissor(0, {scissor});

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_RenderContext->GetGraphicsPipeline(m_TrianglePipelineHandle).pipeline);
    // commandBuffer.draw(3, 1, 0, 0);

    // auto& quadPipeline = m_RenderContext->GetGraphicsPipeline(m_QuadPipelineHandle);

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, quadPipeline.pipeline);

    // vk::DescriptorSet descriptor = m_RenderContext->GetBindGroup(m_UVQuadBindGroup).set;

    // commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, quadPipeline.layout, 0, { descriptor }, {});
    // commandBuffer.draw(6, 1, 0, 0);

    

    auto& globalConstantBuffer = m_RenderContext->GetBuffer(m_GlobalConstantBuffer);

    void* mappedData;
    vmaMapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vmaAllocation, &mappedData);
    auto& globalConstantBufferData = *reinterpret_cast<GlobalConstantBuffer*>(mappedData);
    globalConstantBufferData.vpMatrix = camera->GetViewProjectionMatrix();
    vmaUnmapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vmaAllocation);

    // auto& unlitPipeline = m_RenderContext->GetGraphicsPipeline(m_UnLitPipelineHandle);

    uint32_t dynamicAlignment = 64;

    auto& cubePipeline = m_RenderContext->GetGraphicsPipeline(m_CubePipelineHandle);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, cubePipeline.pipeline);
    auto& globalBindGroup = m_RenderContext->GetBindGroup(m_GlobalBindGroup);
    auto& dynamicBuffer = m_RenderContext->GetDynamicBuffer(m_DynamicBufferHandle);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubePipeline.layout, 0, {globalBindGroup.set}, {});
    for (int i = 0; i < 4; i++)
    {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, cubePipeline.layout, 3, { dynamicBuffer.set}, { i * dynamicAlignment });
        commandBuffer.draw(36, 1, 0, 0);
    }

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

    commandBuffer.endRenderingKHR();

    renderingAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eLoad);
    vk::RenderingInfoKHR imguiRenderInfo({}, vk::Rect2D{{0, 0}, surfaceExtent}, 1, 0, 1, &renderingAttachmentInfo, nullptr);
    commandBuffer.beginRenderingKHR(imguiRenderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

    commandBuffer.endRenderingKHR();

    std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers2;
    imageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
                                      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                                      m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex, swapchainImages[currentSwapchainImageIndex],
                                      vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, imageMemoryBarriers2);

    std::vector<vk::ImageMemoryBarrier> depthImageMemoryBarriers2;
    depthImageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
                                           vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                           m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex,
                                           m_DepthImage,
                                           vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, depthImageMemoryBarriers2);

    commandBuffer.end();

    std::vector<vk::Semaphore> waitSemaphores = {};
    std::vector<vk::PipelineStageFlags> waitStages = {};
    std::vector<vk::CommandBuffer> submitCommandBuffers = { commandBuffer };
    std::vector<vk::Semaphore> renderFinishedSemaphores = { commandElement.semaphore->semaphore };
    vk::SubmitInfo submitInfo(waitSemaphores, waitStages, submitCommandBuffers, renderFinishedSemaphores);
    m_GraphicsQueue.submit({submitInfo}, inFlightFence);

    bool recreated = m_Swapchain.Present(renderFinishedSemaphores, m_PresentQueue);

    if (recreated)
    {
        if (m_DepthImage != nullptr)
        {
            m_DepthImageView = nullptr;
            vmaDestroyImage(m_Device.GetVmaAllocator(), m_DepthImage, m_DepthImageAllocation);
        }
        int width, height;
        window->GetSize(&width, &height);
        CreateDepthBuffer();

        m_Device.SetName(m_Swapchain.Get(), "Main Swapchain");
    }
}

void RenderSystem::Shutdown()
{
    m_Device.WaitIdle();
    
    if (m_DepthImage != nullptr)
    {
        m_DepthImageView = nullptr;

        vmaDestroyImage(m_Device.GetVmaAllocator(), m_DepthImage, m_DepthImageAllocation);
    }
    
    m_RenderDeletionQueue.Flush();

    m_RenderContext->Clear();

    ShutdownImgui();
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
    vk::Extent2D surfaceExtent = m_Swapchain.GetExtent();
    if (surfaceExtent.width == static_cast<uint32_t>(width) && surfaceExtent.height == static_cast<uint32_t>(height))
        return;
    
    m_Device.WaitIdle();
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
	init_info.Queue = *m_GraphicsQueue;
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
            .dynamicBuffer = m_DynamicBufferHandle
        }
    );

    // std::vector<char> unlitVertBytecode = LoadShaderBytecode("sample/UnLit", ShaderStage::Vertex, "vs");
    // std::vector<char> unlitFragBytecode = LoadShaderBytecode("sample/UnLit", ShaderStage::Fragment, "ps");

    // std::vector<char> triangleVertBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Vertex, "vs");
    // std::vector<char> triangleFragBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Fragment, "ps");

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

    // m_TrianglePipelineHandle = m_RenderContext->CreateGraphicsPipeline(
    //     GraphicsPipelineDesc
    //     {
    //         .debugName = "Triangle Pipeline",
    //         .VS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(triangleVertBytecode.data()),
    //             .byteSize = static_cast<uint32_t>(triangleVertBytecode.size()), 
    //             .entryFunc = "vs"
    //         },
    //         .PS
    //         {
    //             .byteCode = reinterpret_cast<uint8_t*>(triangleFragBytecode.data()), 
    //             .byteSize = static_cast<uint32_t>(triangleFragBytecode.size()), 
    //             .entryFunc = "ps"
    //         },            
    //         .colorFormats = {GraphicsFormat::BGRA8_SRGB},
    //         .depthFormat = GraphicsFormat::D32_FLOAT,
    //         .stencilFormat = GraphicsFormat::Undefined,
    //         .bindLayouts {},
    //         .subpassIndex = 0
    //     }
    // );

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
    m_GraphicsQueueFamilyIndex = 0;
    m_PresentQueueFamilyIndex  = 0;

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_Device.GetQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];

        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            m_GraphicsQueueFamilyIndex = i;
            break;
        }
    }

    void* nativeWindowHandle = m_App->GetWindow()->GetNativeHandle();
    for (uint32_t i = queueFamilyProperties.size(); i > 0; --i)
    {
        if (m_Device.GetPhysicalDevice().QueueFamilyIsPresentable(i - 1, nativeWindowHandle))
        {
            m_PresentQueueFamilyIndex = i - 1;
            break;
        }
    }

    m_GraphicsQueue = m_Device.Get().getQueue(m_GraphicsQueueFamilyIndex, 0);
    m_PresentQueue  = m_Device.Get().getQueue(m_PresentQueueFamilyIndex, 0);

    m_Device.SetName(m_GraphicsQueue, "Graphics Queue");
    m_Device.SetName(m_PresentQueue, "Present Queue");
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