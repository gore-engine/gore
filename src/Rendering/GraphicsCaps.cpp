#include "GraphicsCaps.h"

namespace gore::gfx
{
void InitVulkanGraphicsCaps(GraphicsCaps& caps, vk::Instance instance, vk::PhysicalDevice physicalDevice)
{
    vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
    caps.minUniformBufferOffsetAlignment          = deviceProperties.limits.minUniformBufferOffsetAlignment;
}
} // namespace gore::gfx