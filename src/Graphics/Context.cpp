#include "Prefix.h"

#define VMA_IMPLEMENTATION
#include "Context.h"

#include "Rendering/RenderSystem.h"
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

#ifdef ERROR
    #undef ERROR
#endif

namespace gore::graphics
{

Context::Context(RenderSystem* system, App* app) :
    m_RenderSystem(system),
    m_App(app),
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
    m_DepthImageView(nullptr)
{
    CreateInstance();
}

Context::~Context()
{
}

void Context::Initialize()
{
    Window* window = m_App->GetWindow();
    int width, height;
    window->GetSize(&width, &height);

    CreateDevice();
    CreateSurface();
    CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
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

void Context::Update()
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
        CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
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

    vk::ClearValue clearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
    std::vector<vk::ClearValue> clearValues = {clearValue};
    vk::RenderPassBeginInfo renderPassBeginInfo(*m_RenderPass, *m_Framebuffers[m_CurrentSwapchainImageIndex], {{0, 0}, m_SurfaceExtent}, clearValues);
    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);

    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(m_SurfaceExtent.width), static_cast<float>(m_SurfaceExtent.height), 0.0f, 1.0f);
    commandBuffer.setViewport(0, {viewport});

    vk::Rect2D scissor({0, 0}, m_SurfaceExtent);
    commandBuffer.setScissor(0, {scissor});

    std::array<PushConstant, 1> pushConstantData = {pushConstant};
    commandBuffer.pushConstants<PushConstant>(*m_PipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, pushConstantData);

    commandBuffer.draw(36, 1, 0, 0);

    commandBuffer.endRenderPass();

    std::vector<vk::ImageMemoryBarrier> imageMemoryBarriers2;
    imageMemoryBarriers2.emplace_back(vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryRead,
                                     vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
                                      m_GraphicsQueueFamilyIndex, m_GraphicsQueueFamilyIndex,
                                     m_SwapchainImages[m_CurrentSwapchainImageIndex],
                                     vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, imageMemoryBarriers2);

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
        int width, height;
        window->GetSize(&width, &height);
        CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        CreateFramebuffers();
    }
}

void Context::Shutdown()
{
    m_Device.waitIdle();
    if (m_VmaAllocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_VmaAllocator);
    }
}

void Context::OnResize(Window* window, int width, int height)
{
    if (m_SurfaceExtent.width == static_cast<uint32_t>(width) && m_SurfaceExtent.height == static_cast<uint32_t>(height))
        return;

    m_Device.waitIdle();
    m_Swapchain = nullptr;
    CreateSwapchain(3, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    CreateFramebuffers();
}

void Context::CreateInstance()
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

void Context::CreateDevice()
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

void Context::CreateSurface()
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

void Context::CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height)
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

void Context::LoadShader(const std::string& name, const std::string& vertexEntryPoint, const std::string& fragmentEntryPoint)
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

    m_CubeVertexShader           = m_Device.createShaderModule(vertexShaderCreateInfo);
    m_CubeVertexShaderEntryPoint = vertexEntryPoint;

    std::filesystem::path fragmentShaderPath = getShaderFile(vk::ShaderStageFlagBits::eFragment);

    std::vector<char> fragmentShaderBinary = FileSystem::ReadAllBinary(fragmentShaderPath);
    if (fragmentShaderBinary.empty())
    {
        LOG_STREAM(ERROR) << "Failed to load shader: " << fragmentShaderPath << std::endl;
        return;
    }
    LOG_STREAM(DEBUG) << "Loaded shader: " << fragmentShaderPath << std::endl;

    vk::ShaderModuleCreateInfo fragmentShaderCreateInfo({}, fragmentShaderBinary.size(), reinterpret_cast<const uint32_t*>(fragmentShaderBinary.data()));

    m_CubeFragmentShader           = m_Device.createShaderModule(fragmentShaderCreateInfo);
    m_CubeFragmentShaderEntryPoint = fragmentEntryPoint;
}

void Context::CreateRenderPass()
{
    vk::AttachmentDescription colorAttachment({}, m_SurfaceFormat.format,
                                              vk::SampleCountFlagBits::e1,
                                              vk::AttachmentLoadOp::eClear,
                                              vk::AttachmentStoreOp::eStore,
                                              vk::AttachmentLoadOp::eDontCare,
                                              vk::AttachmentStoreOp::eDontCare,
                                              vk::ImageLayout::eColorAttachmentOptimal,
                                              vk::ImageLayout::eColorAttachmentOptimal);

    //    vk::AttachmentDescription depthAttachment({}, vk::Format::eD32Sfloat,
    //                                              vk::SampleCountFlagBits::e1,
    //                                              vk::AttachmentLoadOp::eLoad,
    //                                              vk::AttachmentStoreOp::eStore,
    //                                              vk::AttachmentLoadOp::eLoad,
    //                                              vk::AttachmentStoreOp::eStore,
    //                                              vk::ImageLayout::eDepthStencilAttachmentOptimal,
    //                                              vk::ImageLayout::eDepthStencilAttachmentOptimal);

    std::vector<vk::AttachmentDescription> attachments = {colorAttachment /*, depthAttachment*/};

    vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
    //    vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    std::vector<vk::AttachmentReference> inputAttachmentRefs;
    std::vector<vk::AttachmentReference> colorAttachmentRefs = {colorAttachmentRef};
    std::vector<vk::AttachmentReference> resolveAttachmentRefs;
    std::vector<uint32_t> preserveAttachmentRefs;

    vk::SubpassDescription subpassDesc({}, vk::PipelineBindPoint::eGraphics,
                                       inputAttachmentRefs,
                                       colorAttachmentRefs,
                                       resolveAttachmentRefs,
                                       {}, // depthAttachmentRef,
                                       preserveAttachmentRefs);

    std::vector<vk::SubpassDescription> subpasses = {subpassDesc};
    std::vector<vk::SubpassDependency> dependencies;

    vk::RenderPassCreateInfo renderPassCreateInfo({}, attachments, subpasses, dependencies);

    m_RenderPass = m_Device.createRenderPass(renderPassCreateInfo);
}

void Context::CreatePipeline()
{
    // TODO: this is temporary now!
    vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, 4 * 4 * 2 * sizeof(float));
    std::vector<vk::PushConstantRange> pushConstantRanges = {pushConstantRange};

    // TODO: change this when we have a working descriptor management system
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, {}, pushConstantRanges);

    m_PipelineLayout = m_Device.createPipelineLayout(pipelineLayoutInfo);

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
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo({}, false, false, vk::CompareOp::eLess, false, false, {}, {}, 0.0f, 1.0f);

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

void Context::CreateFramebuffers()
{
    m_Framebuffers.clear();
    m_Framebuffers.reserve(m_SwapchainImageCount);

    for (uint32_t i = 0; i < m_SwapchainImageCount; ++i)
    {
        std::vector<vk::ImageView> attachments = {*m_SwapchainImageViews[i]};
        vk::FramebufferCreateInfo framebufferCreateInfo({}, *m_RenderPass, attachments, m_SurfaceExtent.width, m_SurfaceExtent.height, 1);
        m_Framebuffers.emplace_back(m_Device.createFramebuffer(framebufferCreateInfo));
    }

}

void Context::GetQueues()
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

void Context::CreateCommandPools()
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

bool Context::HasExtension(VulkanInstanceExtension instanceExtension) const
{
    return m_EnabledInstanceExtensions.test(static_cast<size_t>(instanceExtension));
}

bool Context::HasExtension(VulkanDeviceExtension deviceExtension) const
{
    return m_EnabledDeviceExtensions.test(static_cast<size_t>(deviceExtension));
}

int Context::GetScore(int index, const vk::raii::PhysicalDevice& physicalDevice) const
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

void Context::Output(int index, const vk::raii::PhysicalDevice& physicalDevice) const
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
        std::string vendorString = VendorIDToString(static_cast<VendorID>(properties.vendorID));
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

bool Context::QueueFamilyIsPresentable(const vk::raii::PhysicalDevice& physicalDevice,
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

bool Context::QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const
{
    return QueueFamilyIsPresentable(m_PhysicalDevices[m_PhysicalDeviceIndex], queueFamilyIndex, nativeWindowHandle);
}

} // namespace gore::graphics