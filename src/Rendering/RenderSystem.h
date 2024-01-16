#pragma once

#include "Core/System.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include "RenderContext.h"

namespace gore
{

class Window;

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
    std::unique_ptr<RenderContext> m_RenderContext;

    // Instance
    vk::raii::Context m_Context;
    uint32_t m_ApiVersion;
    vk::raii::Instance m_Instance;
    VulkanInstanceExtensionBitset m_EnabledInstanceExtensions;

    // Device
    std::vector<vk::raii::PhysicalDevice> m_PhysicalDevices;
    int m_PhysicalDeviceIndex;
    vk::raii::Device m_Device;
    uint32_t m_DeviceApiVersion;
    VulkanDeviceExtensionBitset m_EnabledDeviceExtensions;

    VmaAllocator m_VmaAllocator;

    // Surface & Swapchain
    vk::raii::SurfaceKHR m_Surface;
    vk::raii::SwapchainKHR m_Swapchain;
    vk::SurfaceFormatKHR m_SurfaceFormat;
    vk::Extent2D m_SurfaceExtent;

    uint32_t m_SwapchainImageCount;

    std::vector<vk::Image> m_SwapchainImages;
    std::vector<vk::raii::ImageView> m_SwapchainImageViews;
    std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
    std::vector<vk::raii::Fence> m_ImageAcquiredFences;
    std::vector<vk::raii::Fence> m_InFlightFences;

    uint32_t m_CurrentSwapchainImageIndex;

    // Shader
    ShaderModuleHandle m_CubeVertexShaderHandle;
    ShaderModuleHandle m_CubeFragmentShaderHandle;

    // Render pass
    vk::raii::RenderPass m_RenderPass;

    // Pipeline
    vk::raii::PipelineLayout m_PipelineLayout;
    vk::raii::Pipeline m_Pipeline;

    // Framebuffer
    std::vector<vk::raii::Framebuffer> m_Framebuffers;

    // Queue
    std::vector<vk::QueueFamilyProperties> m_QueueFamilyProperties;
    vk::raii::Queue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamilyIndex;
    vk::raii::Queue m_PresentQueue;
    uint32_t m_PresentQueueFamilyIndex;

    // Command Pool & Command Buffer
    std::vector<vk::raii::CommandPool> m_CommandPools;
    std::vector<vk::raii::CommandBuffer> m_CommandBuffers;

    // Depth buffer
    vk::Image m_DepthImage;
    VmaAllocation m_DepthImageAllocation;
    vk::raii::ImageView m_DepthImageView;

    vk::raii::Buffer m_VertexBuffer;
    vk::raii::DeviceMemory m_VertexBufferMemory;
    vk::raii::Buffer m_IndexBuffer;
    vk::raii::DeviceMemory m_IndexBufferMemory;

private:
    uint32_t FindMemoryType(uint32_t typeFilter, vk::PhysicalDeviceMemoryProperties memProperties, vk::MemoryPropertyFlags properties) const;

    void CreateInstance();
    void CreateDevice();
    void CreateSurface();
    void CreateSwapchain(uint32_t imageCount, uint32_t width, uint32_t height);
    void CreateDepthBuffer();
    void CreateVertexBuffer();
    void LoadShader(const std::string& name, const std::string& vertexEntryPoint, const std::string& fragmentEntryPoint);
    void CreateRenderPass();
    void CreatePipeline();
    void CreateFramebuffers();
    void GetQueues();
    void CreateCommandPools();

private:
    [[nodiscard]] bool HasExtension(VulkanInstanceExtension instanceExtension) const;
    [[nodiscard]] bool HasExtension(VulkanDeviceExtension deviceExtension) const;
    [[nodiscard]] int GetScore(int index, const vk::raii::PhysicalDevice& physicalDevice) const;
    void Output(int index, const vk::raii::PhysicalDevice& physicalDevice) const;
    [[nodiscard]] bool QueueFamilyIsPresentable(const vk::raii::PhysicalDevice& physicalDevice,
                                                uint32_t queueFamilyIndex,
                                                void* nativeWindowHandle) const;
    [[nodiscard]] bool QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const;
};

} // namespace gore
