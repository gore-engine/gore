#include "Prefix.h"

#include "RenderSystem.h"

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
    // Shader
    m_CubeVertexShader(nullptr),
    m_CubeVertexShaderEntryPoint(),
    m_CubeFragmentShader(nullptr),
    m_CubeFragmentShaderEntryPoint(),
    // Render Pass
    m_RenderPass(nullptr),
    // Pipeline
    m_PipelineLayout(nullptr),
    m_Pipeline(nullptr),
    // Framebuffers
    m_Framebuffers(),
    // Queue
    m_GraphicsQueue(nullptr),
    m_GraphicsQueueFamilyIndex(0),
    m_PresentQueue(nullptr),
    m_PresentQueueFamilyIndex(0),
    // Command Pool & Command Buffer
    m_CommandPool(),
    // Synchronization
    m_RenderFinishedSemaphores(),
    m_InFlightFences(),
    // Depth Buffer
    m_DepthImage(nullptr),
    m_DepthImageAllocation(VK_NULL_HANDLE),
    m_DepthImageView(nullptr)
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    Window* window = m_App->GetWindow();
    int width, height;
    window->GetSize(&width, &height);

    std::vector<gfx::PhysicalDevice> physicalDevices = m_Instance.GetPhysicalDevices();
    m_Device = gfx::Device(GetBestDevice(physicalDevices));

    m_Swapchain = m_Device.CreateSwapchain(window->GetNativeHandle(), 3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    m_Device.SetName(m_Swapchain.Get(), "Main Swapchain");

    CreateDepthBuffer();
    LoadShader("sample/cube", "vs", "ps");
    CreateRenderPass();
    CreatePipeline();
    CreateFramebuffers();
    GetQueues();

    m_CommandPool = m_Device.CreateCommandPool(m_GraphicsQueueFamilyIndex);
    m_Device.SetName(m_CommandPool.Get(0), "CommandPool 0");
    m_Device.SetName(m_CommandPool.Get(1), "CommandPool 1");
    m_Device.SetName(m_CommandPool.Get(2), "CommandPool 2");

    CreateSynchronization();
}

struct PushConstant
{
    Matrix4x4 m;
    Matrix4x4 vp;
};

void RenderSystem::Update()
{
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
    vk::RenderPassBeginInfo renderPassBeginInfo(*m_RenderPass, *m_Framebuffers[currentSwapchainImageIndex], {{0, 0}, surfaceExtent}, clearValues);
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(surfaceExtent.width), static_cast<float>(surfaceExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, {viewport});

    vk::Rect2D scissor({0, 0}, surfaceExtent);
    commandBuffer.setScissor(0, {scissor});

    for (auto& gameObject : Scene::GetActiveScene()->GetGameObjects())
    {
        if (gameObject == camera->GetGameObject())
            continue;

        PushConstant pushConstant
        {
            .m = gameObject->transform->GetLocalToWorldMatrix(),
            .vp = camera->GetViewProjectionMatrix()
        };
        std::array<PushConstant, 1> pushConstantData = {pushConstant};
        commandBuffer.pushConstants<PushConstant>(*m_PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstantData);

        commandBuffer.draw(36, 1, 0, 0);
    }

    commandBuffer.endRenderPass();

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
        CreateFramebuffers();
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
    CreateFramebuffers();
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

void RenderSystem::LoadShader(const std::string& name, const std::string& vertexEntryPoint, const std::string& fragmentEntryPoint)
{
    static const std::filesystem::path kShaderSourceFolder = FileSystem::GetResourceFolder() / "Shaders";

    auto getShaderFile = [&name](vk::ShaderStageFlagBits stage) -> std::filesystem::path
    {
        std::filesystem::path path(name);
        auto shaderPath = kShaderSourceFolder / path.parent_path() / path.filename().stem();
        shaderPath += std::string(".") + (stage == vk::ShaderStageFlagBits::eVertex ? "vert" : "frag") + ".spv";
        return shaderPath;
    };

    std::filesystem::path vertexShaderPath = getShaderFile(vk::ShaderStageFlagBits::eVertex);

    std::vector<char> vertexShaderBinary = FileSystem::ReadAllBinary(vertexShaderPath);
    if (vertexShaderBinary.empty())
    {
        LOG_STREAM(ERROR) << "Failed to load shader: " << vertexShaderPath << std::endl;
        return;
    }
    LOG_STREAM(DEBUG) << "Loaded shader: " << vertexShaderPath << std::endl;

    vk::ShaderModuleCreateInfo vertexShaderCreateInfo({}, vertexShaderBinary.size(), reinterpret_cast<const uint32_t*>(vertexShaderBinary.data()));

    m_CubeVertexShader           = m_Device.Get().createShaderModule(vertexShaderCreateInfo);
    m_CubeVertexShaderEntryPoint = vertexEntryPoint;

    m_Device.SetName(m_CubeVertexShader, "Cube Vertex Shader");

    std::filesystem::path fragmentShaderPath = getShaderFile(vk::ShaderStageFlagBits::eFragment);

    std::vector<char> fragmentShaderBinary = FileSystem::ReadAllBinary(fragmentShaderPath);
    if (fragmentShaderBinary.empty())
    {
        LOG_STREAM(ERROR) << "Failed to load shader: " << fragmentShaderPath << std::endl;
        return;
    }
    LOG_STREAM(DEBUG) << "Loaded shader: " << fragmentShaderPath << std::endl;

    vk::ShaderModuleCreateInfo fragmentShaderCreateInfo({}, fragmentShaderBinary.size(), reinterpret_cast<const uint32_t*>(fragmentShaderBinary.data()));

    m_CubeFragmentShader           = m_Device.Get().createShaderModule(fragmentShaderCreateInfo);
    m_CubeFragmentShaderEntryPoint = fragmentEntryPoint;

    m_Device.SetName(m_CubeFragmentShader, "Cube Fragment Shader");
}

void RenderSystem::CreateRenderPass()
{
    vk::SurfaceFormatKHR surfaceFormat = m_Swapchain.GetFormat();

    vk::AttachmentDescription colorAttachment({}, surfaceFormat.format,
                                              vk::SampleCountFlagBits::e1,
                                              vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eStore,
                                              vk::AttachmentLoadOp::eDontCare,
                                              vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eColorAttachmentOptimal,
                                              vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depthAttachment({}, vk::Format::eD32Sfloat,
                                              vk::SampleCountFlagBits::e1,
                                              vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eDontCare,
                                              vk::AttachmentLoadOp::eDontCare,
                                              vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                              vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::AttachmentDescription> attachments = {colorAttachment, depthAttachment};

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::vector<vk::AttachmentReference> inputAttachmentRefs;
    std::vector<vk::AttachmentReference> colorAttachmentRefs = {colorAttachmentRef};
    std::vector<vk::AttachmentReference> resolveAttachmentRefs;
    std::vector<uint32_t> preserveAttachmentRefs;

    vk::SubpassDescription subpassDesc({}, vk::PipelineBindPoint::eGraphics,
                                       inputAttachmentRefs,
                                       colorAttachmentRefs,
                                       resolveAttachmentRefs,
                                       &depthAttachmentRef,
                                       preserveAttachmentRefs);

    std::vector<vk::SubpassDescription> subpasses = {subpassDesc};
    std::vector<vk::SubpassDependency> dependencies;

    vk::RenderPassCreateInfo renderPassCreateInfo({}, attachments, subpasses, dependencies);

    m_RenderPass = m_Device.Get().createRenderPass(renderPassCreateInfo);

    m_Device.SetName(m_RenderPass, "Cube Color Pass");
}

void RenderSystem::CreatePipeline()
{
    // TODO: this is temporary now!
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstant));
    std::vector<vk::PushConstantRange> pushConstantRanges = {pushConstantRange};

    // TODO: change this when we have a working descriptor management system
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, pushConstantRanges);

    m_PipelineLayout = m_Device.Get().createPipelineLayout(pipelineLayoutInfo);

    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *m_CubeVertexShader, m_CubeVertexShaderEntryPoint.c_str());
    vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *m_CubeFragmentShader, m_CubeFragmentShaderEntryPoint.c_str());
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates);

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, {}, {});
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, false);
    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo({}, 1, nullptr, 1, nullptr);
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false);
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo({}, true, true, vk::CompareOp::eGreaterOrEqual, false, false, {}, {}, 0.0f, 1.0f);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(false,
                                                                    vk::BlendFactor::eOne,
                                                                    vk::BlendFactor::eZero,
                                                                    vk::BlendOp::eAdd,
                                                                    vk::BlendFactor::eOne,
                                                                    vk::BlendFactor::eZero,
                                                                    vk::BlendOp::eAdd,
                                                                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = {colorBlendAttachmentState};

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo({}, false, vk::LogicOp::eCopy, colorBlendAttachmentStates, {0.0f, 0.0f, 0.0f, 0.0f});

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo({},
                                                      shaderStageCreateInfos,
                                                      &vertexInputInfo,
                                                      &inputAssemblyInfo,
                                                      nullptr, // tessellation
                                                      &viewportStateCreateInfo,
                                                      &rasterizationStateCreateInfo,
                                                      &multisampleStateCreateInfo,
                                                      &depthStencilStateCreateInfo,
                                                      &colorBlendStateCreateInfo,
                                                      &dynamicStateCreateInfo,
                                                      *m_PipelineLayout,
                                                      *m_RenderPass,
                                                      0,       // subpass
                                                      nullptr, // basePipelineHandle
                                                      -1);     // basePipelineIndex

    m_Pipeline = m_Device.Get().createGraphicsPipeline(nullptr, pipelineCreateInfo);

    m_Device.SetName(m_Pipeline, "Cube Pipeline");
}

void RenderSystem::CreateFramebuffers()
{
    uint32_t swapchainImageCount                                = m_Swapchain.GetImageCount();
    vk::Extent2D swapchainExtent                                = m_Swapchain.GetExtent();
    const std::vector<vk::raii::ImageView>& swapchainImageViews = m_Swapchain.GetImageViews();

    m_Framebuffers.clear();
    m_Framebuffers.reserve(swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        std::vector<vk::ImageView> attachments = {*swapchainImageViews[i], *m_DepthImageView};
        vk::FramebufferCreateInfo framebufferCreateInfo({}, *m_RenderPass, attachments, swapchainExtent.width, swapchainExtent.height, 1);
        m_Framebuffers.emplace_back(m_Device.Get().createFramebuffer(framebufferCreateInfo));

        m_Device.SetName(m_Framebuffers[i], "Cube Framebuffer " + std::to_string(i));
    }

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

const gfx::PhysicalDevice& RenderSystem::GetBestDevice(const std::vector<gfx::PhysicalDevice>& devices) const
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
