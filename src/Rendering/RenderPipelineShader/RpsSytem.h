#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

#include <memory>

#define RPS_VK_RUNTIME 1
#include "rps/rps.h"

#define AssertIfRpsFailed(expr) (RPS_FAILED(expr) ? assert(false) : (void)0)
#define AssertIfFailed(expr)    (FAILED(expr) ? assert(false) : (void)0)
#define AssertIfFailedEx(expr, errorBlob) \
    ((errorBlob) ? ::OutputDebugStringA((const char*)errorBlob->GetBufferPointer()) : (void)0, AssertIfFailed(expr))

namespace gore::gfx
{
enum RpsQueueType
{
    RPS_QUEUE_GRAPHICS,
    RPS_QUEUE_COMPUTE,
    RPS_QUEUE_COPY,
    RPS_QUEUE_COUNT
};

struct RpsSytemCreateInfo final
{
#if RPS_VK_RUNTIME
    vk::Device device                 = nullptr;
    vk::PhysicalDevice physicalDevice = nullptr;
    void* pUserContext                = nullptr;

    PFN_rpsRuntimeOpRecordDebugMarker pfnRecordDebugMarker = nullptr;
    PFN_rpsRuntimeOpSetDebugName pfnSetDebugName           = nullptr;
#endif // RPS_VK_RUNTIME
};

struct RpsSytem final
{
    std::unique_ptr<RpsDevice> rpsDevice;

    // RenderPipelineShader RenderGraph
    std::unique_ptr<RpsRenderGraph> rpsRDG;

    // IsValid 
    bool IsValid() const
    {
        return rpsDevice != nullptr && rpsRDG != nullptr;
    }
};

std::unique_ptr<RpsSytem> InitializeRpsSystem(const RpsSytemCreateInfo& createInfo);
void DestroyRpsSystem(std::unique_ptr<RpsSytem>& rpsSystem);
} // namespace gore::gfx