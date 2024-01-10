#include "Prefix.h"

#include "Device.h"
#include "Instance.h"
#include "Utils.h"

#include "Core/App.h"
#include "Windowing/Window.h"
#include "Core/Log.h"

#include <vector>
#include <iomanip>
#include <sstream>

namespace gore::gfx
{

PhysicalDevice::PhysicalDevice(const Instance* instance, uint32_t index, const vk::raii::PhysicalDevice& physicalDevice) :
    m_Instance(instance),
    m_Index(index),
    m_PhysicalDevice(physicalDevice)
{

}

PhysicalDevice::~PhysicalDevice()
{

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

} // namespace gore::gfx