#pragma once

#include "Prefix.h"
#include "Handle.h"
#include "GraphicsResourcePrefix.h"
#include "GraphicsFormat.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

namespace gore::gfx
{

enum TextureUsageBits : uint8_t
{
    Sampled      = 1 << 0, // SRV
    Storage      = 1 << 1, // UAV
    RenderTarget = 1 << 2, // RTV
    DepthStencil = 1 << 3, // DSV
    Count
};

FLAG_ENUM_CLASS(TextureUsageBits, uint8_t)

enum class TextureType : uint8_t
{
    Tex1D,
    Tex2D,
    Tex3D,
    Count
};

struct TextureDesc final
{
    const char* debugName  = "Noname Texture_2D_R8G8B8A8_SRGB";
    TextureType type       = TextureType::Tex2D;
    TextureUsageBits usage = TextureUsageBits::Sampled;
    // use dimentions.z for different texturetypes
    // glm::ivec3 dimentions = glm::ivec3(0);
    GraphicsFormat format = GraphicsFormat::RGBA8_SRGB;
    MemoryUsage memUsage  = MemoryUsage::GPU;
    uint32_t width        = 1;
    uint32_t height       = 1;
    uint32_t depth        = 1;
    int numMips           = 1;
    int numLayers         = 1;
    int numSamples        = 1;
    uint8_t* data         = nullptr;
    uint32_t dataSize     = 0;
};

struct Texture
{
    vk::ImageView srv        = VK_NULL_HANDLE;
    vk::ImageView* uav       = nullptr;
    vk::ImageView srvStencil = VK_NULL_HANDLE;

    VkImage image                       = VK_NULL_HANDLE;
    vk::DeviceMemory memory             = VK_NULL_HANDLE;
    vk::DeviceAddress deviceAddress     = 0;
    VmaAllocation vmaAllocation         = VK_NULL_HANDLE;
    VmaAllocationInfo vmaAllocationInfo = {};
};

using TextureHandle = Handle<Texture>;

inline void DestroyVulkanTexture(VmaAllocator vmaAllocator, VkImage image, VmaAllocation vmaAllocation)
{
    vmaDestroyImage(vmaAllocator, image, vmaAllocation);
}

} // namespace gore::gfx