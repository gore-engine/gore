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

#include "Rendering/GPUData/GlobalConstantBuffer.h"
#include "RenderContextHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    // Instance
    m_Instance(app),
    // Device
    m_Device(),
    // Surface & Swapchain
    m_Swapchain(),
    // Pipeline
    m_BlankPipelineLayout(nullptr),
    m_PipelineLayout(nullptr),
    // Queue
    m_GraphicsQueue(nullptr),
    m_GraphicsQueueFamilyIndex(0),
    m_PresentQueue(nullptr),
    m_PresentQueueFamilyIndex(0),
    // Command Pool & Command Buffer
    m_CommandPool(),
    // Global Descriptors
    m_GlobalDescriptorPool(nullptr),
    m_GlobalDescriptorSetLayout(nullptr),
    m_GlobalDescriptorSets(),
    m_GlobalConstantBuffers(),
    // Synchronization
    m_RenderFinishedSemaphores(),
    m_InFlightFences(),
    // Depth Buffer
    m_DepthImage(nullptr),
    m_DepthImageAllocation(VK_NULL_HANDLE),
    m_DepthImageView(nullptr),
    m_VertexBufferHandle(),
    m_IndexBufferHandle(),
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

    m_RenderContext = std::make_unique<RenderContext>(&m_Device);

    CreateDepthBuffer();
    CreateVertexBuffer();
    CreateGlobalDescriptorSets();
    CreateUVQuadDescriptorSets();
    CreatePipeline();
    GetQueues();

    m_CommandPool = m_Device.CreateCommandPool(m_GraphicsQueueFamilyIndex);
    m_Device.SetName(m_CommandPool.Get(0), "CommandPool 0");
    m_Device.SetName(m_CommandPool.Get(1), "CommandPool 1");
    m_Device.SetName(m_CommandPool.Get(2), "CommandPool 2");

    CreateSynchronization();

    InitImgui();
}

struct PushConstant
{
    Matrix4x4 m;
};

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

    vk::Fence inFlightFence = *m_InFlightFences[currentSwapchainImageIndex];

    vk::Result result = m_Device.Get().waitForFences({inFlightFence}, true, UINT64_MAX);
    m_Device.Get().resetFences({inFlightFence});

    m_CommandPool.Reset(currentSwapchainImageIndex);

    const vk::raii::CommandBuffer& commandBuffer = m_CommandPool.GetCommandBuffer(currentSwapchainImageIndex);

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

    // commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_RenderContext->GetGraphicsPipeline(m_TrianglePipelineHandle).pipeline);
    // commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_RenderContext->GetGraphicsPipeline(m_QuadPipelineHandle).pipeline);
    commandBuffer.draw(6, 1, 0, 0);

    auto& globalConstantBuffer = m_RenderContext->GetBuffer(m_GlobalConstantBuffers[currentSwapchainImageIndex]);

    void* mappedData;
    vmaMapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vkBuffer.vmaAllocation, &mappedData);
    auto& globalConstantBufferData = *reinterpret_cast<GlobalConstantBuffer*>(mappedData);
    globalConstantBufferData.vpMatrix = camera->GetViewProjectionMatrix();
    vmaUnmapMemory(m_Device.GetVmaAllocator(), globalConstantBuffer.vkBuffer.vmaAllocation);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_RenderContext->GetGraphicsPipeline(m_CubePipelineHandle).pipeline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_PipelineLayout, 0, {*m_GlobalDescriptorSets[currentSwapchainImageIndex]}, {});

    auto& indexBuffer = m_RenderContext->GetBuffer(m_IndexBufferHandle);
    auto& vertexBuffer = m_RenderContext->GetBuffer(m_VertexBufferHandle);

    for (auto& gameObject : Scene::GetActiveScene()->GetGameObjects())
    {
        if (gameObject == camera->GetGameObject())
            continue;

        PushConstant pushConstant
        {
            .m = gameObject->GetTransform()->GetLocalToWorldMatrix()
        };
        std::array<PushConstant, 1> pushConstantData = {pushConstant};
        commandBuffer.pushConstants<PushConstant>(*m_PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstantData);
        commandBuffer.bindVertexBuffers(0, {vertexBuffer.vkBuffer.vkBuffer}, {0});
        commandBuffer.bindIndexBuffer(indexBuffer.vkBuffer.vkBuffer, 0, vk::IndexType::eUint16);

        commandBuffer.drawIndexed(36, 1, 0, 0, 0);
    }

    commandBuffer.endRenderingKHR();

    renderingAttachmentInfo.setLoadOp(vk::AttachmentLoadOp::eLoad);
    vk::RenderingInfoKHR imguiRenderInfo({}, vk::Rect2D{{0, 0}, surfaceExtent}, 1, 0, 1, &renderingAttachmentInfo, nullptr);
    commandBuffer.beginRenderingKHR(imguiRenderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);

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
    std::vector<vk::CommandBuffer> submitCommandBuffers = {*commandBuffer};
    std::vector<vk::Semaphore> renderFinishedSemaphores = {*m_RenderFinishedSemaphores[currentSwapchainImageIndex]};
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
        CreateSynchronization();

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

    m_RenderContext->clear();

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

TextureHandle RenderSystem::LoadTexture(const std::string& name)
{
    static const std::filesystem::path kTextureFolder = FileSystem::GetResourceFolder() / "Textures";
    auto texturePath = kTextureFolder / name;

    // TODO: change to use std::vector?

    int width, height, channel;
    stbi_uc* pixels = stbi_load(texturePath.generic_string().c_str(), &width, &height, &channel, STBI_rgb_alpha);

    if (pixels == nullptr)
    {
        LOG_STREAM(ERROR) << "RenderSystem LoadTexture: Failed to load texture: " << name << std::endl;
        return TextureHandle();
    }

    TextureHandle handle = m_RenderContext->createTexture({
        .debugName = name.c_str(),
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .data = pixels,
        .dataSize = static_cast<uint32_t>(width * height * 4)
    });

    stbi_image_free(pixels);

    return handle;
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

void RenderSystem::CreateVertexBuffer()
{
    std::vector<Vector3> vertices = {
        Vector3(-1.0f, -1.0f, -1.0f), // 0
        Vector3(-1.0f, -1.0f,  1.0f), // 1
        Vector3(-1.0f,  1.0f, -1.0f), // 2
        Vector3(-1.0f,  1.0f,  1.0f), // 3
        Vector3( 1.0f, -1.0f, -1.0f), // 4
        Vector3( 1.0f, -1.0f,  1.0f), // 5
        Vector3( 1.0f,  1.0f, -1.0f), // 6
        Vector3( 1.0f,  1.0f,  1.0f), // 7
    };

    std::vector<uint16_t> indices = {
        2, 1, 0, 2, 3, 1, // -X
        4, 5, 6, 6, 5, 7, // +X
        0, 1, 5, 0, 5, 4, // -Y
        2, 6, 3, 3, 6, 7, // +Y
        0, 4, 2, 2, 4, 6, // -Z
        1, 3, 5, 5, 3, 7  // +Z
    };

    m_VertexBufferHandle = m_RenderContext->CreateBuffer({
        .debugName = "Vertex Buffer",
        .byteSize = static_cast<uint32_t>(sizeof(Vector3) * vertices.size()),
        .usage = BufferUsage::Vertex,
        .memUsage = MemoryUsage::CPU_TO_GPU
    });

    void* mappedData;
    auto& vertexBuffer = m_RenderContext->GetBuffer(m_VertexBufferHandle);
    vmaMapMemory(m_Device.GetVmaAllocator(), vertexBuffer.vkBuffer.vmaAllocation, &mappedData);
    memcpy(mappedData, vertices.data(), sizeof(Vector3) * vertices.size());
    vmaUnmapMemory(m_Device.GetVmaAllocator(), vertexBuffer.vkBuffer.vmaAllocation);

    m_IndexBufferHandle = m_RenderContext->CreateBuffer({
        .debugName = "Index Buffer",
        .byteSize = static_cast<uint32_t>(sizeof(uint16_t) * indices.size()),
        .usage = BufferUsage::Index,
        .memUsage = MemoryUsage::CPU_TO_GPU
    });

    auto& indexBuffer = m_RenderContext->GetBuffer(m_IndexBufferHandle);

    vmaMapMemory(m_Device.GetVmaAllocator(), indexBuffer.vkBuffer.vmaAllocation, &mappedData);
    memcpy(mappedData, indices.data(), sizeof(uint16_t) * indices.size());
    vmaUnmapMemory(m_Device.GetVmaAllocator(), indexBuffer.vkBuffer.vmaAllocation);

    m_RenderDeletionQueue.PushFunction(
        [&](){
            m_RenderContext->DestroyBuffer(m_VertexBufferHandle);
            m_RenderContext->DestroyBuffer(m_IndexBufferHandle);
        }
    );
}

void RenderSystem::CreateGlobalDescriptorSets()
{
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {       vk::DescriptorType::eUniformBuffer, 10},
        {vk::DescriptorType::eUniformBufferDynamic, 10}
    };

    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 10, poolSizes);
    m_GlobalDescriptorPool = m_Device.Get().createDescriptorPool(descriptorPoolCreateInfo);

    vk::DescriptorSetLayoutBinding globalConstantBufferBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {globalConstantBufferBinding};

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, {bindings});
    m_GlobalDescriptorSetLayout = m_Device.Get().createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

    m_GlobalDescriptorSets.clear();
    m_GlobalDescriptorSets.reserve(m_Swapchain.GetImageCount());
    for (int i = 0; i < m_Swapchain.GetImageCount(); ++i)
    {
        m_GlobalConstantBuffers.emplace_back(m_RenderContext->CreateBuffer({.debugName = "Global Constant Buffer",
                                                                            .byteSize  = sizeof(GlobalConstantBuffer),
                                                                            .usage     = BufferUsage::Uniform,
                                                                            .memUsage  = MemoryUsage::CPU_TO_GPU}));

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(*m_GlobalDescriptorPool, *m_GlobalDescriptorSetLayout);
        vk::raii::DescriptorSets descriptorSets(m_Device.Get(), descriptorSetAllocateInfo);
        m_GlobalDescriptorSets.emplace_back(std::move(descriptorSets[0]));

        vk::DescriptorBufferInfo globalConstantBufferInfo(m_RenderContext->GetBuffer(m_GlobalConstantBuffers[i]).vkBuffer.vkBuffer, 0, sizeof(GlobalConstantBuffer));
        vk::WriteDescriptorSet globalConstantBufferWrite(*m_GlobalDescriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &globalConstantBufferInfo, nullptr);

        m_Device.Get().updateDescriptorSets({globalConstantBufferWrite}, {});
    }

    m_RenderDeletionQueue.PushFunction(
        [&](){
            for (auto& globalConstantBuffer : m_GlobalConstantBuffers)
            {
                m_RenderContext->DestroyBuffer(globalConstantBuffer);
            }
        }
    );
}

void RenderSystem::CreateUVQuadDescriptorSets()
{
    std::vector<vk::DescriptorPoolSize> poolSizes = {
        {       vk::DescriptorType::eUniformBuffer, 10},
        {vk::DescriptorType::eUniformBufferDynamic, 10},
        {       vk::DescriptorType::eCombinedImageSampler, 10}
    };
 
    m_MaterialDescriptorPool = (*m_Device.Get()).createDescriptorPool({vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 10, poolSizes});

    std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        {0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment}
    };

    m_UVQuadDescriptorSetLayout = (*m_Device.Get()).createDescriptorSetLayout({{}, bindings});

    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(m_MaterialDescriptorPool, m_UVQuadDescriptorSetLayout);

    m_UVQuadDescriptorSet = (*m_Device.Get()).allocateDescriptorSets(descriptorSetAllocateInfo).front();

    LOG_STREAM(INFO) << "UV Quad Descriptor Set" << m_UVQuadDescriptorSet << std::endl;

    m_RenderDeletionQueue.PushFunction(
        [&](){
            (*m_Device.Get()).destroyDescriptorPool(m_MaterialDescriptorPool);
            (*m_Device.Get()).destroyDescriptorSetLayout(m_UVQuadDescriptorSetLayout);
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

    std::vector<char> triangleVertBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Vertex, "vs");
    std::vector<char> triangleFragBytecode = LoadShaderBytecode("sample/triangle", ShaderStage::Fragment, "ps");

    std::vector<char> quadVertBytecode = LoadShaderBytecode("sample/quad", ShaderStage::Vertex, "vs");
    std::vector<char> quadFragBytecode = LoadShaderBytecode("sample/quad", ShaderStage::Fragment, "ps");

    // TODO: this is temporary now!
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstant));
    std::vector<vk::PushConstantRange> pushConstantRanges = {pushConstantRange};

    // TODO: change this when we have a working descriptor management system
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, *m_GlobalDescriptorSetLayout, pushConstantRanges);

    m_PipelineLayout = m_Device.Get().createPipelineLayout(pipelineLayoutInfo);
    m_BlankPipelineLayout = m_Device.Get().createPipelineLayout({});

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
            .vertexBufferBindings
            {
                {
                    .byteStride = sizeof(Vector3), 
                    .attributes = 
                    {
                        { .byteOffset = 0, .format = GraphicsFormat::RGB32_FLOAT }
                    }
                }
            },
            .pipelineLayout { *m_PipelineLayout },
            .subpassIndex = 0
        }
    );

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
            .pipelineLayout { *m_BlankPipelineLayout },
            .subpassIndex = 0
        }
    );

    m_QuadPipelineHandle = m_RenderContext->CreateGraphicsPipeline(
        GraphicsPipelineDesc
        {
            .debugName = "UV Quad Pipeline",
            .VS
            {
                .byteCode = reinterpret_cast<uint8_t*>(quadVertBytecode.data()),
                .byteSize = static_cast<uint32_t>(quadVertBytecode.size()), 
                .entryFunc = "vs"
            },
            .PS
            {
                .byteCode = reinterpret_cast<uint8_t*>(quadFragBytecode.data()), 
                .byteSize = static_cast<uint32_t>(quadFragBytecode.size()), 
                .entryFunc = "ps"
            },
            .colorFormats = {GraphicsFormat::BGRA8_SRGB},
            .depthFormat = GraphicsFormat::D32_FLOAT,
            .stencilFormat = GraphicsFormat::Undefined,
            .pipelineLayout { *m_BlankPipelineLayout },
            .subpassIndex = 0
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

void RenderSystem::CreateSynchronization()
{
    m_RenderFinishedSemaphores.clear();
    m_InFlightFences.clear();

    uint32_t imageCount = m_Swapchain.GetImageCount();
    m_RenderFinishedSemaphores.reserve(imageCount);
    m_InFlightFences.reserve(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        m_RenderFinishedSemaphores.emplace_back(m_Device.Get().createSemaphore({}));
        m_InFlightFences.emplace_back(m_Device.Get().createFence({vk::FenceCreateFlagBits::eSignaled}));

        m_Device.SetName(m_RenderFinishedSemaphores[i], "Render Finished Semaphore " + std::to_string(i));
        m_Device.SetName(m_InFlightFences[i], "In Flight Fence " + std::to_string(i));
    }
}

} // namespace gore
