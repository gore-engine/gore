#pragma once

#include "Utilities/Defines.h"

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include <vector>

namespace gore
{

namespace gfx
{

class Instance;
class Device;

class PhysicalDevice
{
public:
    PhysicalDevice();
    PhysicalDevice(const Instance* instance, uint32_t index, vk::raii::PhysicalDevice physicalDevice);
    PhysicalDevice(const PhysicalDevice& other);
    PhysicalDevice(PhysicalDevice&& other) noexcept;
    ~PhysicalDevice();

    PhysicalDevice& operator=(PhysicalDevice&& other) noexcept;

    [[nodiscard]] const vk::raii::PhysicalDevice& Get() const { return m_PhysicalDevice; }

    [[nodiscard]] int Score() const;
    void Output() const;

    [[nodiscard]] bool QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const;

    [[nodiscard]] Device CreateDevice() const;

private:
    friend class Instance;
    friend class Device;
    const Instance* m_Instance;

    uint32_t m_Index;
    vk::raii::PhysicalDevice m_PhysicalDevice;
};

class Device
{
public:
    Device();
    explicit Device(PhysicalDevice physicalDevice);
    Device(Device&& other) noexcept;
    ~Device();

    Device& operator=(Device&& other) noexcept;

    [[nodiscard]] const vk::raii::Device& Get() { return m_Device; }
    [[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
    [[nodiscard]] VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }
    [[nodiscard]] const std::vector<vk::QueueFamilyProperties>& GetQueueFamilyProperties() const { return m_QueueFamilyProperties; }
    [[nodiscard]] uint32_t ApiVersion() const;

    [[nodiscard]] bool HasExtension(VulkanDeviceExtension extension) const;

    void WaitIdle();

private:
    const Instance* m_Instance;
    PhysicalDevice m_PhysicalDevice;

    vk::raii::Device m_Device;
    uint32_t m_DeviceApiVersion;
    VulkanDeviceExtensionBitset m_EnabledDeviceExtensions;

    VmaAllocator m_VmaAllocator;

    std::vector<vk::QueueFamilyProperties> m_QueueFamilyProperties;
};

} // namespace gfx
} // namespace gore
