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
};

std::unique_ptr<RpsSytem> InitializeRpsSystem(const RpsSytemCreateInfo& createInfo);
void DestroyRpsSystem(std::unique_ptr<RpsSytem>& rpsSystem);
} // namespace gore::gfx