#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

// Try to abstract the command ring buffer
// This is a ring buffer that is used to store commands that are sent to the GPU
// Hardware can be optimized to read commands from multiple threads

// HACK: Graphics Command buffer should always be created in the main thread, but can be recorded in multiple threads. We can use a simple ring buffer to store the command buffers and then submit them to the GPU
#define MAX_COMMAND_POOL_PER_RING    64
#define MAX_COMMAND_BUFFERS_PER_POOL 4

namespace gore::gfx
{
struct Semaphore
{
    vk::Semaphore semaphore = VK_NULL_HANDLE;
};

struct Fence
{
    vk::Fence fence = VK_NULL_HANDLE;
};

struct CommandPoolCreateDesc
{
};

struct CommandPool
{
    vk::CommandPool cmdPool = VK_NULL_HANDLE;
};

struct CommandBufferCreateDesc
{
};

struct CommandBuffer1
{
    CommandPool* cmdPool;

    vk::CommandBuffer cmdBuffer;
};

struct CommandRringCreateDesc
{
    uint32_t cmdPoolCount;
    uint32_t cmdBufferPerPoolCount;
};

struct CommandRing
{
    CommandPool* cmdPools[MAX_COMMAND_POOL_PER_RING];
    CommandBuffer1* cmdBuffers[MAX_COMMAND_POOL_PER_RING][MAX_COMMAND_BUFFERS_PER_POOL];
    Fence* fences[MAX_COMMAND_POOL_PER_RING][MAX_COMMAND_BUFFERS_PER_POOL];
    Semaphore* semaphores[MAX_COMMAND_POOL_PER_RING][MAX_COMMAND_BUFFERS_PER_POOL];
    uint32_t currentPoolIndex      = 0;
    uint32_t currentCmdIndex       = 0;
    uint32_t currentFenceIndex     = 0;
    uint32_t poolCount             = 0;
    uint32_t cmdBufferPerPoolCount = 0;
};

struct CommandRingElement
{
    CommandPool* cmdPool       = nullptr;
    CommandBuffer1** cmdBuffer = nullptr;
    Fence* fence               = nullptr;
    Semaphore* semaphore       = nullptr;
    uint32_t cmdBufferCount    = 0;
};
} // namespace gore::gfx