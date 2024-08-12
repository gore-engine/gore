#include "RpsSytem.h"

RPS_DECLARE_RPSL_ENTRY(hello_triangle, main);

namespace gore::gfx
{
std::unique_ptr<RpsSytem> InitializeRpsSystem(const RpsSytemCreateInfo& createInfo)
{
    auto rpsSystem = std::make_unique<RpsSytem>();
#if RPS_VK_RUNTIME
    // Create RPS Device
    RpsVKRuntimeDeviceCreateInfo vkRuntimeDeviceCreateInfo = {};
    vkRuntimeDeviceCreateInfo.hVkDevice                    = createInfo.device;
    vkRuntimeDeviceCreateInfo.hVkPhysicalDevice            = createInfo.physicalDevice;
    vkRuntimeDeviceCreateInfo.flags                        = RPS_VK_RUNTIME_FLAG_NONE;

    RpsRuntimeDeviceCreateInfo rpsCreateInfo     = {};
    rpsCreateInfo.pUserContext                   = createInfo.pUserContext;
    rpsCreateInfo.callbacks.pfnRecordDebugMarker = createInfo.pfnRecordDebugMarker;
    rpsCreateInfo.callbacks.pfnSetDebugName      = createInfo.pfnSetDebugName;

    vkRuntimeDeviceCreateInfo.pRuntimeCreateInfo = &rpsCreateInfo;

    RpsDevice rpsDevice = RPS_NULL_HANDLE;
    AssertIfRpsFailed(rpsVKRuntimeDeviceCreate(&vkRuntimeDeviceCreateInfo, &rpsDevice));

    rpsSystem->rpsDevice = std::make_unique<RpsDevice>(rpsDevice);

    // Create RPS RenderGraph
    RpsRpslEntry entry = RPS_ENTRY_REF(hello_triangle, main);

    RpsRenderGraphCreateInfo renderGraphCreateInfo = {};
    RpsQueueFlags queueFlags[]                     = {RPS_QUEUE_FLAG_GRAPHICS, RPS_QUEUE_FLAG_COMPUTE, RPS_QUEUE_FLAG_COPY};

    renderGraphCreateInfo.scheduleInfo.numQueues              = 3;
    renderGraphCreateInfo.scheduleInfo.pQueueInfos            = queueFlags;
    renderGraphCreateInfo.mainEntryCreateInfo.hRpslEntryPoint = entry;

    RpsRenderGraph rpsRDG = RPS_NULL_HANDLE;
    AssertIfFailed(rpsRenderGraphCreate(rpsDevice, &renderGraphCreateInfo, &rpsRDG));
    
    rpsSystem->rpsRDG = std::make_unique<RpsRenderGraph>(rpsRDG);

#endif
    return rpsSystem;
}

void DestroyRpsSystem(std::unique_ptr<RpsSytem>& rpsSystem)
{
#if RPS_VK_RUNTIME
    if (rpsSystem->rpsRDG)
    {
        rpsRenderGraphDestroy(*rpsSystem->rpsRDG);
    }

    if (rpsSystem->rpsDevice)
    {
        rpsDeviceDestroy(*rpsSystem->rpsDevice);
    }

    rpsSystem.reset();
#endif  
}
} // namespace gore::gfx