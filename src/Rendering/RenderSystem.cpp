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
#if PLATFORM_WIN
    #include "Platform/Windows/Win32Window.h"
#elif PLATFORM_LINUX
    #include "Platform/Linux/X11Window.h"
#elif PLATFORM_MACOS
    #include "Platform/macOS/CocoaWindow.h"
#endif

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
    m_Context(),
    m_ApiVersion(0),
    m_Instance(nullptr),
    m_EnabledInstanceExtensions(),
    // Device
    m_PhysicalDevices(),
    m_PhysicalDeviceIndex(-1),
    m_Device(nullptr),
    m_DeviceApiVersion(0),
    m_EnabledDeviceExtensions(),
    m_VmaAllocator(VK_NULL_HANDLE),
    // Surface & Swapchain
    m_Surface(nullptr),
    m_Swapchain(nullptr),
    m_SurfaceFormat(),
    m_SurfaceExtent(),
    m_SwapchainImageCount(0),
    m_SwapchainImages(),
    m_SwapchainImageViews(),
    m_RenderFinishedSemaphores(),
    m_ImageAcquiredFences(),
    m_InFlightFences(),
    m_CurrentSwapchainImageIndex(0),
    // Shader
    m_CubeVertexShaderHandle(),
    m_CubeFragmentShaderHandle(),
    // Render Pass
    m_RenderPass(nullptr),
    // Pipeline
    m_PipelineLayout(nullptr),
    m_Pipeline(nullptr),
    // Framebuffers
    m_Framebuffers(),
    // Queue
    m_QueueFamilyProperties(),
    m_GraphicsQueue(nullptr),
    m_GraphicsQueueFamilyIndex(0),
    m_PresentQueue(nullptr),
    m_PresentQueueFamilyIndex(0),
    // Command Pool & Command Buffer
    m_CommandPools(),
    m_CommandBuffers(),
    // Depth Buffer
    m_DepthImage(nullptr),
    m_DepthImageAllocation(VK_NULL_HANDLE),
    m_DepthImageView(nullptr),
    m_VertexBuffer(nullptr),
    m_VertexBufferMemory(VK_NULL_HANDLE),
    m_IndexBuffer(nullptr),
    m_IndexBufferMemory(VK_NULL_HANDLE)
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

    CreateInstance();

    CreateDevice();
    CreateSurface();
    CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    CreateDepthBuffer();
    CreateVertexBuffer();
    LoadShader("sample/cube", "vs", "ps");
    CreateRenderPass();
    CreatePipeline();
    CreateFramebuffers();
    GetQueues();
    CreateCommandPools();
}

struct PushConstant
{
    Matrix4x4 model;
    Matrix4x4 proj;
};

void RenderSystem::Update()
{
    Window* window = m_App->GetWindow();
    
    vk::Fence imageAcquiredFence = *m_ImageAcquiredFences[m_CurrentSwapchainImageIndex];
    m_Device.resetFences({imageAcquiredFence});

    auto acquireResult = m_Swapchain.acquireNextImage(UINT64_MAX, nullptr, *m_ImageAcquiredFences[m_CurrentSwapchainImageIndex]);
    vk::Result result = acquireResult.first;
    m_CurrentSwapchainImageIndex = acquireResult.second;
    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
    {
        m_Device.waitIdle();
        m_Swapchain = nullptr;
        int width, height;
        window->GetSize(&width, &height);
        if (m_DepthImage != nullptr)
        {
            m_DepthImageView = nullptr;
            vmaDestroyImage(m_VmaAllocator, m_DepthImage, m_DepthImageAllocation);
        }
        CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        CreateDepthBuffer();
        CreateFramebuffers();
        return;
    }

    vk::Fence inFlightFence = *m_InFlightFences[m_CurrentSwapchainImageIndex];

    result = m_Device.waitForFences({imageAcquiredFence, inFlightFence}, true, UINT64_MAX);
    m_Device.resetFences({inFlightFence});

    float totalTime = GetTotalTime();
    Matrix4x4 camera = Matrix4x4::FromAxisAngle(Vector3::Right, math::constants::PI_4) *
                       Matrix4x4::FromTranslation(Vector3::Up * 2.0f);
    PushConstant pushConstant{
        //        .model = Matrix4x4::CreateTranslation(Vector3::Forward * -2.0f) *
        //                 Matrix4x4::CreateFromQuaternion(Quaternion::CreateFromAxisAngle(Vector3::Right, math::constants::PI_4)),
        .model = Matrix4x4::FromAxisAngle(Vector3::Up, -totalTime) *
                 Matrix4x4::FromTranslation(Vector3::Forward * 2.0f) *
                 camera.Inverse(),
        .proj = Matrix4x4::CreatePerspectiveFieldOfViewLH(math::constants::PI / 3.0f,
                                                          (float)m_SurfaceExtent.width / (float)m_SurfaceExtent.height,
                                                          0.1f, 100.0f)
    };

    vk::raii::CommandPool& commandPool = m_CommandPools[m_CurrentSwapchainImageIndex];
    commandPool.reset({});

    vk::raii::CommandBuffer& commandBuffer = m_CommandBuffers[m_CurrentSwapchainImageIndex];

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    commandBuffer.begin(beginInfo);

    std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers;
    imageMemoryBarriers.emplace_back(vk::AccessFlagBits::eMemoryRead, vk::AccessFlagBits::eColorAttachmentWrite,
                                     vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
                                     m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex,
                                     m_SwapchainImages[m_CurrentSwapchainImageIndex],
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
    vk::ClearValue clearValueDepth(vk::ClearDepthStencilValue(1.0f, 0));
    std::vector<vk::ClearValue> clearValues = {clearValueColor, clearValueDepth};
    vk::RenderPassBeginInfo renderPassBeginInfo(*m_RenderPass, *m_Framebuffers[m_CurrentSwapchainImageIndex], {{0, 0}, m_SurfaceExtent}, clearValues);
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(m_SurfaceExtent.width), static_cast<float>(m_SurfaceExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, {viewport});

    vk::Rect2D scissor({0, 0}, m_SurfaceExtent);
    commandBuffer.setScissor(0, {scissor});

    std::array<PushConstant, 1> pushConstantData = {pushConstant};
    commandBuffer.pushConstants<PushConstant>(*m_PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstantData);

    commandBuffer.bindVertexBuffers(0, {*m_VertexBuffer}, {0});
    commandBuffer.bindIndexBuffer(*m_IndexBuffer, 0, vk::IndexType::eUint16);

    commandBuffer.drawIndexed(36, 1, 0, 0, 0);
    // commandBuffer.draw(36, 1, 0, 0);

    commandBuffer.endRenderPass();

    std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers2;
    imageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
                                      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                                      m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex,
                                      m_SwapchainImages[m_CurrentSwapchainImageIndex],
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
    std::vector<vk::Semaphore> renderFinishedSemaphores = {*m_RenderFinishedSemaphores[m_CurrentSwapchainImageIndex]};
    vk::SubmitInfo submitInfo(waitSemaphores, waitStages, submitCommandBuffers, renderFinishedSemaphores);
    m_GraphicsQueue.submit({submitInfo}, inFlightFence);

    vk::PresentInfoKHR presentInfo(renderFinishedSemaphores, *m_Swapchain, m_CurrentSwapchainImageIndex);
    result = m_PresentQueue.presentKHR(presentInfo);

    if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR)
    {
        m_Device.waitIdle();
        m_Swapchain = nullptr;
        if (m_DepthImage != nullptr)
        {
            m_DepthImageView = nullptr;
            vmaDestroyImage(m_VmaAllocator, m_DepthImage, m_DepthImageAllocation);
        }
        int width, height;
        window->GetSize(&width, &height);
        CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        CreateDepthBuffer();
        CreateFramebuffers();
    }
}

void RenderSystem::Shutdown()
{
    m_Device.waitIdle();
    
    if (m_DepthImage != nullptr)
    {
        m_DepthImageView = nullptr;

        vmaDestroyImage(m_VmaAllocator, m_DepthImage, m_DepthImageAllocation);
    }

    if (m_VmaAllocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_VmaAllocator);
    }
    m_RenderContext->clear();
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
    if (m_SurfaceExtent.width == static_cast<uint32_t>(width) && m_SurfaceExtent.height == static_cast<uint32_t>(height))
        return;
    
    m_Device.waitIdle();
    m_Swapchain = nullptr;
    if (m_DepthImage != nullptr)
    {
        m_DepthImageView = nullptr;
        vmaDestroyImage(m_VmaAllocator, m_DepthImage, m_DepthImageAllocation);
    }
    CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    CreateDepthBuffer();
    CreateFramebuffers();
}

void RenderSystem::CreateInstance()
{
    // Layers
    std::vector<vk::LayerProperties> layerProperties = m_Context.enumerateInstanceLayerProperties();
    
    std::vector<const char*> requestedLayers = {
#if ENGINE_DEBUG
        "VK_LAYER_KHRONOS_validation",
#endif
    };

    bool validationEnabled = false;
    std::vector<const char*> enabledLayers;
    for (const auto& requestedLayer : requestedLayers)
    {
        auto it = std::find_if(layerProperties.begin(), layerProperties.end(), [&requestedLayer](const vk::LayerProperties& layer)
                               { return strcmp(layer.layerName, requestedLayer) == 0; });

        if (it != layerProperties.end())
        {
            enabledLayers.push_back(it->layerName);
            validationEnabled = validationEnabled || strcmp(it->layerName, "VK_LAYER_KHRONOS_validation") == 0;
        }
    }

    if (!enabledLayers.empty())
    {
        LOG(DEBUG, "Enabled instance layers:\n");
        for (const auto& layer : enabledLayers)
        {
            LOG(DEBUG, "  %s\n", layer);
        }
    }

    // Instance Extensions
    std::vector<vk::ExtensionProperties> instanceExtensionProperties = m_Context.enumerateInstanceExtensionProperties();

    for (std::string layer : enabledLayers)
    {
        std::vector<vk::ExtensionProperties> layerExtensionProperties = m_Context.enumerateInstanceExtensionProperties(layer);
        instanceExtensionProperties.insert(instanceExtensionProperties.end(), layerExtensionProperties.begin(), layerExtensionProperties.end());
    }

    m_EnabledInstanceExtensions.set();
#ifndef ENGINE_DEBUG
    m_EnabledInstanceExtensions.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_report));
    m_EnabledInstanceExtensions.reset(static_cast<size_t>(VulkanInstanceExtension::kVK_EXT_debug_utils));
#endif

    std::vector<const char*> enabledInstanceExtensions = BuildEnabledExtensions<VulkanInstanceExtensionBitset, VulkanInstanceExtension>(instanceExtensionProperties,
                                                                                                                                        m_EnabledInstanceExtensions);

    // Instance Creation
#ifdef VK_API_VERSION_1_1
    m_ApiVersion = m_Context.enumerateInstanceVersion(); // TODO: potential crash
#else
    m_ApiVersion = VK_API_VERSION_1_0;
#endif

    vk::ApplicationInfo appInfo("Gore", 1, "Gore", 1, m_ApiVersion);
    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, enabledLayers, enabledInstanceExtensions);

#if defined(VK_EXT_validation_features) && (VK_EXT_VALIDATION_FEATURES_SPEC_VERSION + 0) >= 4

    // todo: make this a setting
    const bool enableShaderPrintf = true;

    std::vector<vk::ValidationFeatureEnableEXT> enabledValidationFeatures;
    std::vector<vk::ValidationFeatureDisableEXT> disabledValidationFeatures;
    vk::ValidationFeaturesEXT validationFeatures;

    if (validationEnabled)
    {
        if (enableShaderPrintf)
        {
            enabledValidationFeatures.push_back(vk::ValidationFeatureEnableEXT::eDebugPrintf);
        }

        validationFeatures.setEnabledValidationFeatures(enabledValidationFeatures);
        validationFeatures.setDisabledValidationFeatures(disabledValidationFeatures);

        instanceCreateInfo.setPNext(&validationFeatures);
    }
#endif

#if defined(VK_KHR_portability_enumeration)
    if (m_EnabledInstanceExtensions.test(static_cast<size_t>(VulkanInstanceExtension::kVK_KHR_portability_enumeration)))
        instanceCreateInfo.flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    m_Instance = m_Context.createInstance(instanceCreateInfo);

    LOG_STREAM(INFO) << "Created Vulkan instance version "
                     << VK_API_VERSION_MAJOR(m_ApiVersion) << "."
                     << VK_API_VERSION_MINOR(m_ApiVersion) << "."
                     << VK_API_VERSION_PATCH(m_ApiVersion) << std::endl;
}

void RenderSystem::CreateDevice()
{
    // Physical Device
    m_PhysicalDevices = m_Instance.enumeratePhysicalDevices();

    int maxScore = -1;
    for (int i = 0; i < m_PhysicalDevices.size(); ++i)
    {
        const vk::raii::PhysicalDevice& physicalDevice = m_PhysicalDevices[i];
        Output(i, physicalDevice);

        int score = GetScore(i, physicalDevice);
        if (score >= maxScore)
        {
            maxScore              = score;
            m_PhysicalDeviceIndex = i;
        }
    }

    if (m_PhysicalDeviceIndex < 0)
    {
        LOG_STREAM(FATAL) << "No suitable physical device found" << std::endl;
        return;
    }

    // Device
    const vk::raii::PhysicalDevice& physicalDevice = m_PhysicalDevices[m_PhysicalDeviceIndex];
    vk::PhysicalDeviceProperties properties        = physicalDevice.getProperties();
    m_DeviceApiVersion                             = properties.apiVersion;

    // Queue Families
    m_QueueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(m_QueueFamilyProperties.size());
    std::vector<std::vector<float>> queuePriorities;
    queuePriorities.reserve(m_QueueFamilyProperties.size());

    for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = m_QueueFamilyProperties[i];

        std::vector<float> queuePriority(queueFamilyProperty.queueCount, 1.0f);
        queuePriorities.push_back(queuePriority);

        vk::DeviceQueueCreateInfo queueCreateInfo({}, i, queuePriorities[i]);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Features
    vk::PhysicalDeviceFeatures enabledFeatures = physicalDevice.getFeatures();

    // Device extensions
    std::vector<vk::ExtensionProperties> deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
    m_EnabledDeviceExtensions.set();
    std::vector<const char*> enabledDeviceExtensions = BuildEnabledExtensions<VulkanDeviceExtensionBitset, VulkanDeviceExtension>(deviceExtensionProperties,
                                                                                                                                  m_EnabledDeviceExtensions);

    // Create
    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {}, enabledDeviceExtensions, &enabledFeatures);
    m_Device = physicalDevice.createDevice(deviceCreateInfo);

    m_RenderContext = std::make_unique<RenderContext>(&m_Device);

    LOG_STREAM(INFO) << "Created Vulkan device with \"" << properties.deviceName << "\"" << std::endl;

    // Create Vulkan Memory Allocator
    VmaVulkanFunctions vulkanFunctions{
        .vkGetInstanceProcAddr = m_Instance.getDispatcher()->vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr   = m_Device.getDispatcher()->vkGetDeviceProcAddr
    };
    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags                       = 0,              // TODO: check what flags we can use potentially
        .physicalDevice              = *physicalDevice,
        .device                      = *m_Device,
        .preferredLargeHeapBlockSize = 0,              // TODO: we are using default value here for now
        .pAllocationCallbacks        = VK_NULL_HANDLE,
        .pDeviceMemoryCallbacks      = VK_NULL_HANDLE,
        .pHeapSizeLimit              = VK_NULL_HANDLE, // TODO: this means no limit on all heaps
        .pVulkanFunctions            = &vulkanFunctions,
        .instance                    = *m_Instance,
        .vulkanApiVersion            = std::min(m_ApiVersion, m_DeviceApiVersion)
    };

    VkResult res = vmaCreateAllocator(&allocatorCreateInfo, &m_VmaAllocator);
    VK_CHECK_RESULT(res);
}

void RenderSystem::CreateSurface()
{
    void* nativeWindowHandle = m_App->GetWindow()->GetNativeHandle();
#ifdef VK_KHR_win32_surface

    Win32Window* win32Window = reinterpret_cast<Win32Window*>(nativeWindowHandle);
    vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo({}, win32Window->hInstance, win32Window->hWnd);
    m_Surface = m_Instance.createWin32SurfaceKHR(surfaceCreateInfo);

#elif VK_KHR_xlib_surface

    X11Window* x11Window = reinterpret_cast<X11Window*>(nativeWindowHandle);
    vk::XlibSurfaceCreateInfoKHR surfaceCreateInfo({}, x11Window->display, x11Window->window);
    m_Surface = m_Instance.createXlibSurfaceKHR(surfaceCreateInfo);

#elif VK_EXT_metal_surface

    CocoaWindow* cocoaWindow = reinterpret_cast<CocoaWindow*>(nativeWindowHandle);
    vk::MetalSurfaceCreateInfoEXT surfaceCreateInfo({}, cocoaWindow->layer);
    m_Surface = m_Instance.createMetalSurfaceEXT(surfaceCreateInfo);

#else
    #error "No supported surface extension available"
#endif

    LOG(DEBUG, "Created Vulkan surface\n");
}

void RenderSystem::CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height)
{
    m_SwapchainImages.clear();
    m_SwapchainImageViews.clear();
    m_RenderFinishedSemaphores.clear();
    m_ImageAcquiredFences.clear();
    m_InFlightFences.clear();

    const vk::raii::PhysicalDevice& physicalDevice = m_PhysicalDevices[m_PhysicalDeviceIndex];

    vk::SurfaceCapabilitiesKHR surfaceCapabilities               = physicalDevice.getSurfaceCapabilitiesKHR(*m_Surface);
    std::vector<vk::SurfaceFormatKHR> surfaceSupportedFormats    = physicalDevice.getSurfaceFormatsKHR(*m_Surface);
    std::vector<vk::PresentModeKHR> surfaceSupportedPresentModes = physicalDevice.getSurfacePresentModesKHR(*m_Surface);

    if (surfaceSupportedFormats.empty() || surfaceSupportedPresentModes.empty())
    {
        LOG(ERROR, "Physical device does not support swapchain\n");
        return;
    }

    m_SurfaceFormat = surfaceSupportedFormats[0];
    for (const auto& format : surfaceSupportedFormats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            m_SurfaceFormat = format;
            break;
        }
    }

    vk::PresentModeKHR surfacePresentMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode : surfaceSupportedPresentModes)
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            surfacePresentMode = presentMode;
            break;
        }
    }

    m_SurfaceExtent.width  = std::clamp(width,
                                       surfaceCapabilities.minImageExtent.width,
                                       surfaceCapabilities.maxImageExtent.width);
    m_SurfaceExtent.height = std::clamp(height,
                                        surfaceCapabilities.minImageExtent.height,
                                        surfaceCapabilities.maxImageExtent.height);
    uint32_t layers        = std::min(1u, surfaceCapabilities.maxImageArrayLayers); // TODO: Require layers

    if (surfaceCapabilities.maxImageCount < surfaceCapabilities.minImageCount)
    {
        // fix for some drivers
        std::swap(surfaceCapabilities.maxImageCount, surfaceCapabilities.minImageCount);
    }

    m_SwapchainImageCount = std::clamp(imageCount,
                                       surfaceCapabilities.minImageCount,
                                       surfaceCapabilities.maxImageCount);

    m_CurrentSwapchainImageIndex = 0;

    vk::SwapchainCreateInfoKHR createInfo({}, *m_Surface, m_SwapchainImageCount,
                                          m_SurfaceFormat.format, m_SurfaceFormat.colorSpace,
                                          m_SurfaceExtent, layers,
                                          vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, // TODO: Check supported usage and maybe get this from user?
                                          vk::SharingMode::eExclusive,
                                          {}, // queueFamilies
                                          surfaceCapabilities.currentTransform,
                                          vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                          surfacePresentMode,
                                          true,     // clipped
                                          nullptr); // oldSwapchain

    m_Swapchain = m_Device.createSwapchainKHR(createInfo);

    m_SwapchainImages = m_Swapchain.getImages();

    m_SwapchainImageViews.reserve(m_SwapchainImageCount);
    m_RenderFinishedSemaphores.reserve(m_SwapchainImageCount);
    m_ImageAcquiredFences.reserve(m_SwapchainImageCount);
    m_InFlightFences.reserve(m_SwapchainImageCount);

    for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo({}, m_SwapchainImages[i], vk::ImageViewType::e2D, m_SurfaceFormat.format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        m_SwapchainImageViews.emplace_back(m_Device.createImageView(imageViewCreateInfo));
        m_RenderFinishedSemaphores.emplace_back(m_Device.createSemaphore({}));
        m_ImageAcquiredFences.emplace_back(m_Device.createFence({vk::FenceCreateFlagBits::eSignaled}));
        m_InFlightFences.emplace_back(m_Device.createFence({vk::FenceCreateFlagBits::eSignaled}));
    }

    LOG(DEBUG, "Created Vulkan swapchain with %d images, size %dx%d\n", m_SwapchainImageCount, m_SurfaceExtent.width, m_SurfaceExtent.height);
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

    const vk::raii::PhysicalDevice& physicalDevice = m_PhysicalDevices[m_PhysicalDeviceIndex];

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

    vk::ImageCreateInfo imageCreateInfo({}, vk::ImageType::e2D, depthFormat,
                                        vk::Extent3D(m_SurfaceExtent.width, m_SurfaceExtent.height, 1),
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
    VkResult res = vmaCreateImage(m_VmaAllocator, cImageCreateInfo, &allocationCreateInfo, depthImage, &m_DepthImageAllocation, nullptr);
    VK_CHECK_RESULT(res);

    vk::ImageViewCreateInfo imageViewCreateInfo({},
                                                m_DepthImage,
                                                vk::ImageViewType::e2D,
                                                depthFormat,
                                                {},
                                                {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

    m_DepthImageView = m_Device.createImageView(imageViewCreateInfo);
}

uint32_t RenderSystem::FindMemoryType(uint32_t typeFilter, vk::PhysicalDeviceMemoryProperties memProperties, vk::MemoryPropertyFlags properties) const
{
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type");
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

    // create a vk::raii::Buffer vertexBuffer, given a vk::raii::Device device and some vertexData in host memory
    vk::BufferCreateInfo bufferCreateInfo( {}, sizeof(Vector3) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer );
    m_VertexBuffer = m_Device.createBuffer( bufferCreateInfo );
    
    // create a vk::raii::DeviceMemory vertexDeviceMemory, given a vk::raii::Device device and a uint32_t memoryTypeIndex
    vk::MemoryRequirements memoryRequirements = m_VertexBuffer.getMemoryRequirements();
    uint32_t memoryTypeIndex = FindMemoryType( memoryRequirements.memoryTypeBits, m_PhysicalDevices[m_PhysicalDeviceIndex].getMemoryProperties(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
    
    vk::MemoryAllocateInfo memoryAllocateInfo( memoryRequirements.size, memoryTypeIndex );
    m_VertexBufferMemory = m_Device.allocateMemory( memoryAllocateInfo );

    // bind the complete device memory to the vertex buffer
    m_VertexBuffer.bindMemory( *m_VertexBufferMemory, 0 );

    // copy the vertex data into the vertexDeviceMemory
    uint8_t* pData = static_cast<uint8_t*>(m_VertexBufferMemory.mapMemory( 0, memoryRequirements.size ));
    memcpy( pData, vertices.data(), sizeof(Vector3) * vertices.size() );
    m_VertexBufferMemory.unmapMemory();

    // create a vk::raii::Buffer indexBuffer, given a vk::raii::Device device and some indexData in host memory
    vk::BufferCreateInfo indexBufferCreateInfo( {}, sizeof(uint16_t) * indices.size(), vk::BufferUsageFlagBits::eIndexBuffer );
    m_IndexBuffer = m_Device.createBuffer( indexBufferCreateInfo );
    
    // create a vk::raii::DeviceMemory indexDeviceMemory, given a vk::raii::Device device and a uint32_t memoryTypeIndex
    memoryRequirements = m_IndexBuffer.getMemoryRequirements();
    memoryTypeIndex = FindMemoryType( memoryRequirements.memoryTypeBits, m_PhysicalDevices[m_PhysicalDeviceIndex].getMemoryProperties(), vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );

    memoryAllocateInfo = vk::MemoryAllocateInfo( memoryRequirements.size, memoryTypeIndex );
    m_IndexBufferMemory = m_Device.allocateMemory( memoryAllocateInfo );
    
    m_IndexBuffer.bindMemory( *m_IndexBufferMemory, 0 );

    // copy the index data into the indexDeviceMemory
    pData = static_cast<uint8_t*>(m_IndexBufferMemory.mapMemory( 0, memoryRequirements.size ));
    memcpy( pData, indices.data(), sizeof(uint16_t) * indices.size() );
    m_IndexBufferMemory.unmapMemory();
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

    m_CubeVertexShaderHandle = m_RenderContext->createShaderModule({
        .debugName = "Cube Vertex Shader",
        .byteCode = reinterpret_cast<uint8_t*>(vertexShaderBinary.data()),
        .byteSize = static_cast<uint32_t>(vertexShaderBinary.size()),
        .entryFunc = vertexEntryPoint.c_str()
    });

    vk::ShaderModuleCreateInfo vertexShaderCreateInfo({}, vertexShaderBinary.size(), reinterpret_cast<const uint32_t*>(vertexShaderBinary.data()));

    std::filesystem::path fragmentShaderPath = getShaderFile(vk::ShaderStageFlagBits::eFragment);

    std::vector<char> fragmentShaderBinary = FileSystem::ReadAllBinary(fragmentShaderPath);
    if (fragmentShaderBinary.empty())
    {
        LOG_STREAM(ERROR) << "Failed to load shader: " << fragmentShaderPath << std::endl;
        return;
    }
    LOG_STREAM(DEBUG) << "Loaded shader: " << fragmentShaderPath << std::endl;

    vk::ShaderModuleCreateInfo fragmentShaderCreateInfo({}, fragmentShaderBinary.size(), reinterpret_cast<const uint32_t*>(fragmentShaderBinary.data()));

    m_CubeFragmentShaderHandle = m_RenderContext->createShaderModule({
        .debugName = "Cube Frag Shader",
        .byteCode = reinterpret_cast<uint8_t*>(fragmentShaderBinary.data()),
        .byteSize = static_cast<uint32_t>(fragmentShaderBinary.size()),
        .entryFunc = fragmentEntryPoint.c_str()
    });
}

void RenderSystem::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachment({}, m_SurfaceFormat.format,
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

    m_RenderPass = m_Device.createRenderPass(renderPassCreateInfo);
}

void RenderSystem::CreatePipeline()
{
    // TODO: this is temporary now!
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, 4 * 4 * 2 * sizeof(float));
    std::vector<vk::PushConstantRange> pushConstantRanges = {pushConstantRange};

    // TODO: change this when we have a working descriptor management system
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, pushConstantRanges);

    m_PipelineLayout = m_Device.createPipelineLayout(pipelineLayoutInfo);

    auto& vertexShaderModuleDesc = m_RenderContext->getShaderModuleDesc(m_CubeVertexShaderHandle);
    auto& vertexShaderModule = m_RenderContext->getShaderModule(m_CubeVertexShaderHandle);

    auto& fragmentShaderModuleDesc = m_RenderContext->getShaderModuleDesc(m_CubeFragmentShaderHandle);
    auto& fragmentShaderModule = m_RenderContext->getShaderModule(m_CubeFragmentShaderHandle);

    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, *vertexShaderModule.sm, vertexShaderModuleDesc.entryFunc);
    vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, *fragmentShaderModule.sm, fragmentShaderModuleDesc.entryFunc);
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({}, dynamicStates);

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, false);
    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo({}, 1, nullptr, 1, nullptr);
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo({}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false);
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo({}, true, true, vk::CompareOp::eLess, false, false, {}, {}, 0.0f, 1.0f);

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

    m_Pipeline = m_Device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
}

void RenderSystem::CreateFramebuffers()
{
    m_Framebuffers.clear();
    m_Framebuffers.reserve(m_SwapchainImageCount);

    for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
    {
        std::vector<vk::ImageView> attachments = {*m_SwapchainImageViews[i], *m_DepthImageView};
        vk::FramebufferCreateInfo framebufferCreateInfo({}, *m_RenderPass, attachments, m_SurfaceExtent.width, m_SurfaceExtent.height, 1);
        m_Framebuffers.emplace_back(m_Device.createFramebuffer(framebufferCreateInfo));
    }

}

void RenderSystem::GetQueues()
{
    m_GraphicsQueueFamilyIndex = 0;
    m_PresentQueueFamilyIndex  = 0;

    for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = m_QueueFamilyProperties[i];

        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            m_GraphicsQueueFamilyIndex = i;
            break;
        }
    }

    void* nativeWindowHandle = m_App->GetWindow()->GetNativeHandle();
    for (uint32_t i = m_QueueFamilyProperties.size(); i > 0; --i)
    {
        if (QueueFamilyIsPresentable(m_PhysicalDevices[m_PhysicalDeviceIndex], i - 1, nativeWindowHandle))
        {
            m_PresentQueueFamilyIndex = i - 1;
            break;
        }
    }

    m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueFamilyIndex, 0);
    m_PresentQueue  = m_Device.getQueue(m_PresentQueueFamilyIndex, 0);
}

void RenderSystem::CreateCommandPools()
{
    m_CommandPools.reserve(m_SwapchainImageCount);
    m_CommandBuffers.reserve(m_SwapchainImageCount);
    vk::CommandPoolCreateInfo commandPoolCreateInfo({}, m_GraphicsQueueFamilyIndex);
    for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
    {
        m_CommandPools.emplace_back(m_Device.createCommandPool(commandPoolCreateInfo));
        std::vector<vk::raii::CommandBuffer> buffers = m_Device.allocateCommandBuffers({*m_CommandPools[i], vk::CommandBufferLevel::ePrimary, 1});
        m_CommandBuffers.emplace_back(nullptr);
        m_CommandBuffers[i].swap(buffers[0]);
    }
}

bool RenderSystem::HasExtension(VulkanInstanceExtension instanceExtension) const
{
    return m_EnabledInstanceExtensions.test(static_cast<size_t>(instanceExtension));
}

bool RenderSystem::HasExtension(VulkanDeviceExtension deviceExtension) const
{
    return m_EnabledDeviceExtensions.test(static_cast<size_t>(deviceExtension));
}

int RenderSystem::GetScore(int index, const vk::raii::PhysicalDevice& physicalDevice) const
{
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    int score                               = 0;

    // TODO: more criteria

    // score by device type
    switch (properties.deviceType)
    {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            score += 1000;
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            score += 500;
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            score += 250;
            break;
        case vk::PhysicalDeviceType::eCpu:
            score += 100;
            break;
        default:
            break;
    }

    // score by index, smaller index is better
    score -= index;

    return score;
}

std::string GetHumanReadableDeviceSize(vk::DeviceSize size)
{
    std::stringstream result;
    const float kKB = 1024.0f;
    const float kMB = kKB * 1024.0f;
    const float kGB = kMB * 1024.0f;

    const auto sizef = static_cast<float>(size);

    if (sizef < kKB)
        result << size << " Bytes";
    else if (sizef < kMB)
        result << std::fixed << std::setprecision(1) << sizef / kKB << " KB";
    else if (sizef < kGB)
        result << std::fixed << std::setprecision(1) << sizef / kMB << " MB";
    else
        result << std::fixed << std::setprecision(1) << sizef / kGB << " GB";

    return result.str();
}

void RenderSystem::Output(int index, const vk::raii::PhysicalDevice& physicalDevice) const
{
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    LOG_STREAM(INFO) << "Adapter #" << index << ": \"" << properties.deviceName << "\"" << std::endl;

#ifdef ENGINE_DEBUG
    vk::PhysicalDeviceMemoryProperties memoryProperties          = physicalDevice.getMemoryProperties();
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    std::vector<bool> queueFamilySupportsPresent;
    queueFamilySupportsPresent.resize(queueFamilyProperties.size());
    for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        queueFamilySupportsPresent[i] = QueueFamilyIsPresentable(physicalDevice, static_cast<uint32_t>(i), m_App->GetWindow()->GetNativeHandle());
    }

    auto logStream = LOG_STREAM(DEBUG);
    {
        std::string vendorString = VendorIDToString(static_cast<gfx::VendorID>(properties.vendorID));
        logStream << "    Vendor ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.vendorID << " [" << vendorString << "]" << std::endl;
    }
    logStream << "    Device ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.deviceID << std::endl;
    logStream << "    Device Type: " << to_string(properties.deviceType) << std::endl;
    logStream << "    Device driver API Version: " << std::resetiosflags(std::ios_base::basefield)
              << VK_API_VERSION_MAJOR(properties.apiVersion) << "."
              << VK_API_VERSION_MINOR(properties.apiVersion) << "."
              << VK_API_VERSION_PATCH(properties.apiVersion) << std::endl;

    // Queue Families
    logStream << "    Queue Families:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];
        logStream << "        #" << i << ": Queue Count: " << queueFamilyProperty.queueCount << std::endl;
        logStream << "            Queue Flags: " << to_string(queueFamilyProperty.queueFlags) << std::endl;
        logStream << "            Presentable: " << (queueFamilySupportsPresent[i] ? "Yes" : "No") << std::endl;
        logStream << "            Timestamp Valid Bits: " << queueFamilyProperty.timestampValidBits << std::endl;
        logStream << "            Min Image Transfer Granularity: ("
                  << queueFamilyProperty.minImageTransferGranularity.width << ", "
                  << queueFamilyProperty.minImageTransferGranularity.height << ", "
                  << queueFamilyProperty.minImageTransferGranularity.depth << ")" << std::endl;
    }

    // Memory Heaps
    logStream << "    Memory Heaps:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
    {
        const vk::MemoryHeap& memoryHeap = memoryProperties.memoryHeaps[i];
        logStream << "        #" << i << ": " << GetHumanReadableDeviceSize(memoryHeap.size) << (memoryHeap.flags & vk::MemoryHeapFlagBits::eDeviceLocal ? ", Device Local" : "") << std::endl;
    }

    // Memory Types
    logStream << "    Memory Types:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        const vk::MemoryType& memoryType = memoryProperties.memoryTypes[i];
        logStream << "        #" << i << ": Heap #" << memoryType.heapIndex << ", " << to_string(memoryType.propertyFlags) << std::endl;
    }
#endif
}

bool RenderSystem::QueueFamilyIsPresentable(const vk::raii::PhysicalDevice& physicalDevice,
                                       uint32_t queueFamilyIndex,
                                       void* nativeWindowHandle) const
{
#ifdef VK_KHR_win32_surface
    if (HasExtension(VulkanInstanceExtension::kVK_KHR_win32_surface))
    {
        vk::Bool32 presentSupport = physicalDevice.getWin32PresentationSupportKHR(queueFamilyIndex);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_KHR_xlib_surface
    if (HasExtension(VulkanInstanceExtension::kVK_KHR_xlib_surface))
    {
        auto* x11Window           = static_cast<X11Window*>(nativeWindowHandle);
        vk::Bool32 presentSupport = physicalDevice.getXlibPresentationSupportKHR(queueFamilyIndex, *x11Window->display, x11Window->visualID);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_EXT_metal_surface
    return HasExtension(VulkanInstanceExtension::kVK_EXT_metal_surface);
#endif

    return false;
}

bool RenderSystem::QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const
{
    return QueueFamilyIsPresentable(m_PhysicalDevices[m_PhysicalDeviceIndex], queueFamilyIndex, nativeWindowHandle);
}

} // namespace gore
