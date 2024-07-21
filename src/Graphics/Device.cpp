#include "Prefix.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "Utils.h"

#include "Core/App.h"
#include "Windowing/Window.h"
#include "Core/Log.h"
#if PLATFORM_WIN
    #include "Platform/Windows/Win32Window.h"
#elif PLATFORM_LINUX
    #include "Platform/Linux/X11Window.h"
#elif PLATFORM_MACOS
    #include "Platform/macOS/CocoaWindow.h"
#endif

#include <utility>
#include <vector>
#include <iomanip>
#include <sstream>

namespace gore::gfx
{

PhysicalDevice::PhysicalDevice() :
    m_Instance(nullptr),
    m_Index(UINT32_MAX),
    m_PhysicalDevice(nullptr)
{
}

PhysicalDevice::PhysicalDevice(const Instance* instance, uint32_t index, vk::raii::PhysicalDevice physicalDevice) :
    m_Instance(instance),
    m_Index(index),
    m_PhysicalDevice(std::move(physicalDevice))
{
}

PhysicalDevice::PhysicalDevice(PhysicalDevice&& other) noexcept :
    m_Instance(other.m_Instance),
    m_Index(other.m_Index),
    m_PhysicalDevice(std::move(other.m_PhysicalDevice))
{
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice& other) :
    m_Instance(other.m_Instance),
    m_Index(other.m_Index),
    m_PhysicalDevice(other.m_PhysicalDevice)
{
}

PhysicalDevice::~PhysicalDevice()
{
}

PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&& other) noexcept
{
    m_Instance          = other.m_Instance;
    m_Index             = other.m_Index;
    m_PhysicalDevice    = std::move(other.m_PhysicalDevice);

    return *this;
}

int PhysicalDevice::Score() const
{
    vk::PhysicalDeviceProperties properties = m_PhysicalDevice.getProperties();
    int score                               = 0;

    // TODO: more criteria

    // score by device type
    switch (properties.deviceType)
    {
        case vk::PhysicalDeviceType::eDiscreteGpu:
            score += 1000;
            break;
        case vk::PhysicalDeviceType::eIntegratedGpu:
            score += 500;
            break;
        case vk::PhysicalDeviceType::eVirtualGpu:
            score += 250;
            break;
        case vk::PhysicalDeviceType::eCpu:
            score += 100;
            break;
        default:
            break;
    }

    // score by index, smaller index is better
    score -= m_Index;

    return score;
}

std::string GetHumanReadableDeviceSize(vk::DeviceSize size)
{
    std::stringstream result;
    const float kKB = 1024.0f;
    const float kMB = kKB * 1024.0f;
    const float kGB = kMB * 1024.0f;

    const auto sizef = static_cast<float>(size);

    if (sizef < kKB)
        result << size << " Bytes";
    else if (sizef < kMB)
        result << std::fixed << std::setprecision(1) << sizef / kKB << " KB";
    else if (sizef < kGB)
        result << std::fixed << std::setprecision(1) << sizef / kMB << " MB";
    else
        result << std::fixed << std::setprecision(1) << sizef / kGB << " GB";

    return result.str();
}

void PhysicalDevice::Output() const
{
    vk::PhysicalDeviceProperties properties = m_PhysicalDevice.getProperties();
    LOG_STREAM(INFO) << "Adapter #" << m_Index << ": \"" << properties.deviceName << "\"" << std::endl;

#ifdef ENGINE_DEBUG
    vk::PhysicalDeviceMemoryProperties memoryProperties          = m_PhysicalDevice.getMemoryProperties();
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();

    std::vector<bool> queueFamilySupportsPresent;
    queueFamilySupportsPresent.resize(queueFamilyProperties.size());
    for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        queueFamilySupportsPresent[i] = QueueFamilyIsPresentable(static_cast<uint32_t>(i), m_Instance->GetApp()->GetWindow()->GetNativeHandle());
    }

    auto logStream = LOG_STREAM(DEBUG);
    {
        std::string vendorString = VendorIDToString(static_cast<gfx::VendorID>(properties.vendorID));
        logStream << "    Vendor ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.vendorID << " [" << vendorString << "]" << std::endl;
    }
    logStream << "    Device ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.deviceID << std::endl;
    logStream << "    Device Type: " << to_string(properties.deviceType) << std::endl;
    logStream << "    Device driver API Version: " << std::resetiosflags(std::ios_base::basefield)
              << VK_API_VERSION_MAJOR(properties.apiVersion) << "."
              << VK_API_VERSION_MINOR(properties.apiVersion) << "."
              << VK_API_VERSION_PATCH(properties.apiVersion) << std::endl;

    // Queue Families
    logStream << "    Queue Families:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];
        logStream << "        #" << i << ": Queue Count: " << queueFamilyProperty.queueCount << std::endl;
        logStream << "            Queue Flags: " << to_string(queueFamilyProperty.queueFlags) << std::endl;
        logStream << "            Presentable: " << (queueFamilySupportsPresent[i] ? "Yes" : "No") << std::endl;
        logStream << "            Timestamp Valid Bits: " << queueFamilyProperty.timestampValidBits << std::endl;
        logStream << "            Min Image Transfer Granularity: ("
                  << queueFamilyProperty.minImageTransferGranularity.width << ", "
                  << queueFamilyProperty.minImageTransferGranularity.height << ", "
                  << queueFamilyProperty.minImageTransferGranularity.depth << ")" << std::endl;
    }

    // Memory Heaps
    logStream << "    Memory Heaps:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
    {
        const vk::MemoryHeap& memoryHeap = memoryProperties.memoryHeaps[i];
        logStream << "        #" << i << ": " << GetHumanReadableDeviceSize(memoryHeap.size) << (memoryHeap.flags & vk::MemoryHeapFlagBits::eDeviceLocal ? ", Device Local" : "") << std::endl;
    }

    // Memory Types
    logStream << "    Memory Types:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        const vk::MemoryType& memoryType = memoryProperties.memoryTypes[i];
        logStream << "        #" << i << ": Heap #" << memoryType.heapIndex << ", " << to_string(memoryType.propertyFlags) << std::endl;
    }
#endif
}

bool PhysicalDevice::QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const
{
#ifdef VK_KHR_win32_surface
    if (m_Instance->HasExtension(VulkanInstanceExtension::kVK_KHR_win32_surface))
    {
        vk::Bool32 presentSupport = m_PhysicalDevice.getWin32PresentationSupportKHR(queueFamilyIndex);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_KHR_xlib_surface
    if (m_Instance->HasExtension(VulkanInstanceExtension::kVK_KHR_xlib_surface))
    {
        auto* x11Window           = static_cast<X11Window*>(nativeWindowHandle);
        vk::Bool32 presentSupport = m_PhysicalDevice.getXlibPresentationSupportKHR(queueFamilyIndex, *x11Window->display, x11Window->visualID);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_EXT_metal_surface
    return m_Instance->HasExtension(VulkanInstanceExtension::kVK_EXT_metal_surface);
#endif

    return false;
}

Device PhysicalDevice::CreateDevice() const
{
    Device device(*this);
    return device;
}

Device::Device() :
    m_Instance(nullptr),
    m_PhysicalDevice(),
    m_Device(nullptr),
    m_DeviceApiVersion(0),
    m_EnabledDeviceExtensions(),
    m_VmaAllocator(VK_NULL_HANDLE),
    m_QueueFamilyProperties()
{
}

Device::Device(PhysicalDevice physicalDevice) :
    m_Instance(nullptr),
    m_PhysicalDevice(std::move(physicalDevice)),
    m_Device(nullptr),
    m_DeviceApiVersion(0),
    m_EnabledDeviceExtensions(),
    m_VmaAllocator(VK_NULL_HANDLE),
    m_QueueFamilyProperties()
{
    m_Instance                              = m_PhysicalDevice.m_Instance;
    const vk::raii::PhysicalDevice& pd      = m_PhysicalDevice.Get();

    vk::PhysicalDeviceProperties properties = pd.getProperties();
    m_DeviceApiVersion                      = properties.apiVersion;

    // Queue Families
    m_QueueFamilyProperties = pd.getQueueFamilyProperties();

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(m_QueueFamilyProperties.size());
    std::vector<std::vector<float>> queuePriorities;
    queuePriorities.reserve(m_QueueFamilyProperties.size());

    for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
    {
        const vk::QueueFamilyProperties& queueFamilyProperty = m_QueueFamilyProperties[i];

        std::vector<float> queuePriority(queueFamilyProperty.queueCount, 1.0f);
        queuePriorities.push_back(queuePriority);

        vk::DeviceQueueCreateInfo queueCreateInfo({}, i, queuePriorities[i]);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Features
    vk::PhysicalDeviceFeatures2 enabledFeatures2 = pd.getFeatures2();

    vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;    

    bufferDeviceAddressFeatures.pNext = &dynamicRenderingFeatures;

    enabledFeatures2.pNext = &bufferDeviceAddressFeatures;

    // Device extensions
    std::vector<vk::ExtensionProperties> deviceExtensionProperties = pd.enumerateDeviceExtensionProperties();
    m_EnabledDeviceExtensions.set();
    std::vector<const char*> enabledDeviceExtensions = BuildEnabledExtensions<VulkanDeviceExtensionBitset, VulkanDeviceExtension>(deviceExtensionProperties,
                                                                                                                                  m_EnabledDeviceExtensions);                                                                                                                                
    // Create
    vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {}, enabledDeviceExtensions, nullptr, &enabledFeatures2);
    m_Device = pd.createDevice(deviceCreateInfo);

    SetName(m_Device, properties.deviceName);

    LOG_STREAM(INFO) << "Created Vulkan device with \"" << properties.deviceName << "\"" << std::endl;

    // Create Vulkan Memory Allocator
    VmaVulkanFunctions vulkanFunctions{
        .vkGetInstanceProcAddr = m_Instance->Get().getDispatcher()->vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr   = m_Device.getDispatcher()->vkGetDeviceProcAddr
    };
    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags                       = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,              // TODO: check what flags we can use potentially
        .physicalDevice              = *pd,
        .device                      = *m_Device,
        .preferredLargeHeapBlockSize = 0,              // TODO: we are using default value here for now
        .pAllocationCallbacks        = VK_NULL_HANDLE,
        .pDeviceMemoryCallbacks      = VK_NULL_HANDLE,
        .pHeapSizeLimit              = VK_NULL_HANDLE, // TODO: this means no limit on all heaps
        .pVulkanFunctions            = &vulkanFunctions,
        .instance                    = *m_Instance->Get(),
        .vulkanApiVersion            = std::min(m_Instance->Version(), m_DeviceApiVersion)
    };

    VkResult res = vmaCreateAllocator(&allocatorCreateInfo, &m_VmaAllocator);
    VK_CHECK_RESULT(res);
}

Device::Device(Device&& other) noexcept :
    m_Instance(other.m_Instance),
    m_PhysicalDevice(std::move(other.m_PhysicalDevice)),
    m_Device(std::move(other.m_Device)),
    m_DeviceApiVersion(other.m_DeviceApiVersion),
    m_EnabledDeviceExtensions(other.m_EnabledDeviceExtensions),
    m_VmaAllocator(std::exchange(other.m_VmaAllocator, VK_NULL_HANDLE)),
    m_QueueFamilyProperties(std::move(other.m_QueueFamilyProperties))
{
}

Device::~Device()
{
    if (*m_Device != nullptr)
    {
        m_Device.waitIdle();
    }

    if (m_VmaAllocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_VmaAllocator);
    }
}

Device& Device::operator=(Device&& other) noexcept
{
    m_Instance                  = other.m_Instance;
    m_PhysicalDevice            = std::move(other.m_PhysicalDevice);
    m_Device                    = std::move(other.m_Device);
    m_DeviceApiVersion          = other.m_DeviceApiVersion;
    m_EnabledDeviceExtensions   = other.m_EnabledDeviceExtensions;
    m_VmaAllocator              = std::exchange(other.m_VmaAllocator, VK_NULL_HANDLE);
    m_QueueFamilyProperties     = std::move(other.m_QueueFamilyProperties);

    return *this;
}

uint32_t Device::GetQueueFamilyIndexByFlags(vk::QueueFlags flags) const
{
    for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); ++i)
    {
        if ((m_QueueFamilyProperties[i].queueFlags & flags) == flags)
        {
            return i;
        }
    }

    LOG_STREAM(FATAL) << "Failed to find queue family with flags: " << to_string(flags) << std::endl;
    throw std::runtime_error("Failed to find queue family with flags");
}

uint32_t Device::ApiVersion() const
{
    return std::min(m_Instance->Version(), m_DeviceApiVersion);
}

bool Device::HasExtension(VulkanDeviceExtension extension) const
{
    return m_EnabledDeviceExtensions.test(static_cast<size_t>(extension));
}

void Device::WaitIdle() const
{
    m_Device.waitIdle();
}

Swapchain Device::CreateSwapchain(void* nativeWindowHandle, uint32_t imageCount, uint32_t width, uint32_t height) const
{
    return gfx::Swapchain(*this, nativeWindowHandle, imageCount, width, height);
}

void Device::SetName(uint64_t objectHandle, vk::ObjectType objectType, const std::string& name) const
{
#ifdef ENGINE_DEBUG

    bool hasDebugUtils = false;
    bool hasDebugReport = false;

#if defined(VK_EXT_debug_utils)
    hasDebugUtils = m_Instance->HasExtension(VulkanInstanceExtension::kVK_EXT_debug_utils);
#endif

#if defined(VK_EXT_debug_marker)
    hasDebugReport = HasExtension(VulkanDeviceExtension::kVK_EXT_debug_marker);
#endif

    if (hasDebugUtils)
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfoUtils(objectType, objectHandle, name.c_str());
        m_Device.setDebugUtilsObjectNameEXT(nameInfoUtils);
    }
    else if (hasDebugReport)
    {
        vk::DebugMarkerObjectNameInfoEXT nameInfoMarker(debugReportObjectType(objectType), objectHandle, name.c_str());
        m_Device.debugMarkerSetObjectNameEXT(nameInfoMarker);
    }
#endif
}

} // namespace gore::gfx