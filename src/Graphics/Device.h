#pragma once

#include "Utilities/Defines.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

namespace gore
{

namespace gfx
{

class Instance;

class PhysicalDevice
{
public:
    PhysicalDevice(const Instance* instance, uint32_t index, const vk::raii::PhysicalDevice& physicalDevice);
    ~PhysicalDevice();

    [[nodiscard]] const vk::raii::PhysicalDevice& Get() const { return m_PhysicalDevice; }

    [[nodiscard]] int Score() const;
    void Output() const;

    [[nodiscard]] bool QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const;

private:
    friend class Instance;
    const Instance* m_Instance;

    uint32_t m_Index;
    vk::raii::PhysicalDevice m_PhysicalDevice;
};

class Device
{
public:
    Device();
    ~Device();

    NON_COPYABLE(Device);
};

} // namespace gfx
} // namespace gore
