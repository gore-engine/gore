#pragma once

#include "Prefix.h"

#include "Graphics/Vulkan/VulkanIncludes.h"

#include <memory>

#define RPS_VK_RUNTIME 1
#include "rps/rps.h"

namespace gore::gfx
{
struct RPSSytemCreateInfo final
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

std::unique_ptr<RpsSytem> InitializeRPSSystem(const RPSSytemCreateInfo& createInfo);
} // namespace gore::gfx