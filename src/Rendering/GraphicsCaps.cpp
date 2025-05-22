#include "GraphicsCaps.h"
#include "Graphics/Graphics.h"

namespace gore::gfx
{
void InitVulkanGraphicsCaps(GraphicsCaps& caps, Instance& instance, Device& device)
{
    uint32_t vulkanMinorVersion = VK_API_VERSION_MINOR(instance.Version());

    vk::PhysicalDeviceProperties deviceProperties = device.GetPhysicalDevice().Get().getProperties();
    caps.minUniformBufferOffsetAlignment          = deviceProperties.limits.minUniformBufferOffsetAlignment;

    // Check for bindless support, which is available in Vulkan 1.2 and later
    // Note: This is a simplified check. In a real application, you would want to check for specific features
    // but if the device cannot support vulkan 1.2, it cannot support bindless.
    caps.supportsBindless = vulkanMinorVersion >= 2;
}
} // namespace gore::gfx