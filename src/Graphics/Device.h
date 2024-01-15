#pragma once

#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include <vector>
#include <type_traits>

namespace gore::gfx
{

template <typename T>
concept VulkanRAIIType = requires {
    typename T::CType;
    typename T::CppType;
    { T::objectType } -> std::same_as<const vk::ObjectType&>;
};

class Instance;
class Device;
class Swapchain;
class CommandPool;

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

    [[nodiscard]] const vk::raii::Device& Get() const { return m_Device; }
    [[nodiscard]] const PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
    [[nodiscard]] const Instance* GetInstance() const { return m_Instance; }
    [[nodiscard]] VmaAllocator GetVmaAllocator() const { return m_VmaAllocator; }
    [[nodiscard]] const std::vector<vk::QueueFamilyProperties>& GetQueueFamilyProperties() const { return m_QueueFamilyProperties; }
    [[nodiscard]] uint32_t ApiVersion() const;

    [[nodiscard]] bool HasExtension(VulkanDeviceExtension extension) const;

    void WaitIdle() const;

    [[nodiscard]] Swapchain CreateSwapchain(void* nativeWindowHandle, uint32_t imageCount, uint32_t width, uint32_t height) const;
    [[nodiscard]] CommandPool CreateCommandPool(uint32_t queueFamilyIndex) const;

    template<typename VkRAIIObjectType>
    void SetName(const VkRAIIObjectType& object, const std::string& name) const
    {
        static_assert(VulkanRAIIType<VkRAIIObjectType>, "<object> must be a Vulkan RAII type.");
#ifdef ENGINE_DEBUG
        auto vkObject = static_cast<typename VkRAIIObjectType::CType>(*object);
        SetName(reinterpret_cast<uint64_t>(vkObject), VkRAIIObjectType::objectType, name);
#endif
    }
    void SetName(uint64_t objectHandle, vk::ObjectType objectType, const std::string& name) const;

private:
    const Instance* m_Instance;
    PhysicalDevice m_PhysicalDevice;

    vk::raii::Device m_Device;
    uint32_t m_DeviceApiVersion;
    VulkanDeviceExtensionBitset m_EnabledDeviceExtensions;

    VmaAllocator m_VmaAllocator;

    std::vector<vk::QueueFamilyProperties> m_QueueFamilyProperties;
};

} // namespace gore::gfx
