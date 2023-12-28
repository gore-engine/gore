#include "Prefix.h"

#include "VulkanDevice.h"

#include "Core/App.h"
#include "Core/Log.h"
#include "Rendering/RenderUtils.h"
#include "Windowing/Window.h"
#if PLATFORM_LINUX
    #include "Platform/Linux/X11Window.h"
#endif

#include <utility>
#include <iomanip>

namespace gore
{

static std::string DeviceTypeToString(VkPhysicalDeviceType deviceType);
static std::string QueueFlagsToString(VkQueueFlags queueFlags);
static std::string GetHumanReadableDeviceSize(VkDeviceSize size);
static std::string MemoryPropertyTypeToString(VkMemoryPropertyFlags memoryPropertyFlags);

VulkanPhysicalDevice::VulkanPhysicalDevice(VulkanInstance* instance, int index, VkPhysicalDevice vkPhysicalDevice) :
    instance(instance),
    index(index),
    physicalDevice(vkPhysicalDevice),
    properties(),
    features(),
    memoryProperties(),
    queueFamilyProperties(),
    queueFamilySupportsPresent(),
    extensionProperties()
{
    // Device properties
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    // Device features
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    // Memory properties
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);

    // Queue family properties
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyPropertyCount, VK_NULL_HANDLE);

    queueFamilyProperties.resize(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice,
                                             &queueFamilyPropertyCount,
                                             queueFamilyProperties.data());

    // Queue presentable
    queueFamilySupportsPresent.resize(queueFamilyPropertyCount);
    for (uint32_t i = 0; i < queueFamilyPropertyCount; ++i)
    {
        queueFamilySupportsPresent[i] = QueueFamilyIsPresentable(i, instance->GetApp()->GetWindow()->GetNativeHandle());
    }

    // Device extension properties
    uint32_t extensionPropertyCount = 0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, VK_NULL_HANDLE, &extensionPropertyCount, VK_NULL_HANDLE);

    extensionProperties.resize(extensionPropertyCount);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice,
                                         VK_NULL_HANDLE,
                                         &extensionPropertyCount,
                                         extensionProperties.data());
}

void VulkanPhysicalDevice::LogInfo() const
{
    LOG_STREAM(INFO) << "Adapter #" << index << ": \"" << properties.deviceName << "\"" << std::endl;

#ifdef ENGINE_DEBUG
    auto logStream = LOG_STREAM(DEBUG);
    {
        std::string vendorString = VendorIDToString(static_cast<VendorID>(properties.vendorID));
        logStream << "    Vendor ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.vendorID << " [" << vendorString << "]" << std::endl;
    }
    logStream << "    Device ID: 0x" << std::hex << std::setw(4) << std::setfill('0') << properties.deviceID << std::endl;
    logStream << "    Device Type: " << DeviceTypeToString(properties.deviceType) << std::endl;
    logStream << "    Device driver API Version: " << std::resetiosflags(std::ios_base::basefield)
              << VK_API_VERSION_MAJOR(properties.apiVersion) << "."
              << VK_API_VERSION_MINOR(properties.apiVersion) << "."
              << VK_API_VERSION_PATCH(properties.apiVersion) << std::endl;

    // Queue Families
    logStream << "    Queue Families:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
    {
        const VkQueueFamilyProperties& queueFamilyProperty = queueFamilyProperties[i];
        logStream << "        #" << i << ": Queue Count: " << queueFamilyProperty.queueCount << std::endl;
        logStream << "            Queue Flags: " << QueueFlagsToString(queueFamilyProperty.queueFlags) << std::endl;
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
        const VkMemoryHeap& memoryHeap = memoryProperties.memoryHeaps[i];
        logStream << "        #" << i << ": " << GetHumanReadableDeviceSize(memoryHeap.size) << (memoryHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ? ", Device Local" : "") << std::endl;
    }

    // Memory Types
    logStream << "    Memory Types:" << std::endl;
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        const VkMemoryType& memoryType = memoryProperties.memoryTypes[i];
        logStream << "        #" << i << ": Heap #" << memoryType.heapIndex << ", " << MemoryPropertyTypeToString(memoryType.propertyFlags) << std::endl;
    }
#endif
}

int VulkanPhysicalDevice::Score() const
{
    int score = 0;

    // TODO: more criteria

    // score by device type
    switch (properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            score += 1000;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            score += 500;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            score += 250;
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            score += 100;
            break;
        default:
            break;
    }

    // score by index, smaller index is better
    score -= index;

    return score;
}

bool VulkanPhysicalDevice::QueueFamilyIsPresentable(uint32_t queueFamilyIndex, void* nativeWindowHandle) const
{
#ifdef VK_KHR_win32_surface
    if (instance->HasExtension(VulkanInstanceExtension::kVK_KHR_win32_surface))
    {
        VkBool32 presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_KHR_xlib_surface
    if (instance->HasExtension(VulkanInstanceExtension::kVK_KHR_xlib_surface))
    {
        auto* x11Window = static_cast<X11Window*>(nativeWindowHandle);
        VkBool32 presentSupport = vkGetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, x11Window->display, x11Window->visualID);
        return presentSupport == VK_TRUE;
    }
#endif

#ifdef VK_EXT_metal_surface
    return instance->HasExtension(VulkanInstanceExtension::kVK_EXT_metal_surface);
#endif

    return false;
}

VulkanDevice::VulkanDevice(VulkanInstance* instance, VulkanPhysicalDevice physicalDevice) :
    API(),
    m_Instance(instance),
    m_Device(VK_NULL_HANDLE),
    m_PhysicalDevice(std::move(physicalDevice)),
    m_EnabledExtensions(),
    m_Queues()
{
    // Create device
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext              = nullptr;
    deviceCreateInfo.flags              = 0;

    // Queue create info
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(m_PhysicalDevice.queueFamilyProperties.size());
    for (uint32_t i = 0; i < m_PhysicalDevice.queueFamilyProperties.size(); ++i)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext                   = nullptr;
        queueCreateInfo.flags                   = 0;
        queueCreateInfo.queueFamilyIndex        = i;
        queueCreateInfo.queueCount              = m_PhysicalDevice.queueFamilyProperties[i].queueCount;
        queueCreateInfo.pQueuePriorities        = nullptr; // TODO: we might want to set this for things like distinguishing between upload/download queues, high/low priority compute jobs, etc.

        queueCreateInfos.push_back(queueCreateInfo);
    }

    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();

    // Device features
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures       = &deviceFeatures;

    // Device extensions
    std::vector<const char*> enabledDeviceExtensions = GetEnabledDeviceExtensions(m_EnabledExtensions,
                                                                                  m_PhysicalDevice.extensionProperties);

    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();

    // Create device
    VkResult res = vkCreateDevice(m_PhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_Device);
    VK_CHECK_RESULT(res);
    if (res != VK_SUCCESS)
    {
        LOG(ERROR, "Failed to create Vulkan device\n");
        return;
    }
    LOG_STREAM(INFO) << "Created Vulkan device with \"" << m_PhysicalDevice.properties.deviceName << "\"" << std::endl;

    // Load device functions
    volkLoadDeviceTable(&API, m_Device);

    // Get queues
    m_Queues.resize(m_PhysicalDevice.queueFamilyProperties.size());
    for (uint32_t i = 0; i < m_PhysicalDevice.queueFamilyProperties.size(); ++i)
    {
        m_Queues[i].resize(m_PhysicalDevice.queueFamilyProperties[i].queueCount);
        for (uint32_t j = 0; j < m_PhysicalDevice.queueFamilyProperties[i].queueCount; ++j)
        {
            API.vkGetDeviceQueue(m_Device, i, j, &m_Queues[i][j]);
        }
    }
}

VulkanDevice::~VulkanDevice()
{
    if (m_Device != VK_NULL_HANDLE)
    {
        API.vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
        LOG_STREAM(INFO) << "Destroyed Vulkan device" << std::endl;
    }
}

bool VulkanDevice::HasExtension(VulkanDeviceExtension extension) const
{
    return m_EnabledExtensions.test(static_cast<size_t>(extension));
}

std::string DeviceTypeToString(VkPhysicalDeviceType deviceType)
{
    switch (deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "Other";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        default:
            return "Unknown";
    }
}

std::string QueueFlagsToString(VkQueueFlags queueFlags)
{
    std::stringstream ss;

    int bitCount = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (queueFlags & (1 << i))
        {
            if (bitCount > 0)
                ss << " | ";

            switch (1 << i)
            {
                case VK_QUEUE_GRAPHICS_BIT:
                    ss << "Graphics";
                    break;
                case VK_QUEUE_COMPUTE_BIT:
                    ss << "Compute";
                    break;
                case VK_QUEUE_TRANSFER_BIT:
                    ss << "Transfer";
                    break;
                case VK_QUEUE_SPARSE_BINDING_BIT:
                    ss << "Sparse Binding";
                    break;
#ifdef VK_VERSION_1_1
                case VK_QUEUE_PROTECTED_BIT:
                    ss << "Protected";
                    break;
#endif
#ifdef VK_KHR_video_decode_queue
                case VK_QUEUE_VIDEO_DECODE_BIT_KHR:
                    ss << "Video Decode";
                    break;
#endif
#if defined(VK_KHR_video_encode_queue) && defined(VK_ENABLE_BETA_EXTENSIONS)
                case VK_QUEUE_VIDEO_ENCODE_BIT_KHR:
                    ss << "Video Encode";
                    break;
#endif
#ifdef VK_NV_optical_flow
                case VK_QUEUE_OPTICAL_FLOW_BIT_NV:
                    ss << "Optical Flow (NV)";
                    break;
#endif
                default:
                    ss << "Unknown";
                    break;
            }

            ++bitCount;
        }
    }

    if (bitCount == 0)
        ss << "None";

    return ss.str();
}

std::string GetHumanReadableDeviceSize(VkDeviceSize size)
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

std::string MemoryPropertyTypeToString(VkMemoryPropertyFlags memoryPropertyFlags)
{
    std::stringstream ss;

    int bitCount = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (memoryPropertyFlags & (1 << i))
        {
            if (bitCount > 0)
                ss << " | ";

            switch (1 << i)
            {
                case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:
                    ss << "Device Local";
                    break;
                case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:
                    ss << "Host Visible";
                    break;
                case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:
                    ss << "Host Coherent";
                    break;
                case VK_MEMORY_PROPERTY_HOST_CACHED_BIT:
                    ss << "Host Cached";
                    break;
                case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:
                    ss << "Lazily Allocated";
                    break;
#ifdef VK_VERSION_1_1
                case VK_MEMORY_PROPERTY_PROTECTED_BIT:
                    ss << "Protected";
                    break;
#endif
#ifdef VK_AMD_device_coherent_memory
                case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD:
                    ss << "Device Coherent (AMD)";
                    break;
                case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD:
                    ss << "Device Uncached (AMD)";
                    break;
#endif
#ifdef VK_NV_external_memory_rdma
                case VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV:
                    ss << "RDMA Capable (NV)";
                    break;
#endif
                default:
                    ss << "Unknown";
                    break;
            }

            ++bitCount;
        }
    }

    if (bitCount == 0)
        ss << "None";

    return ss.str();
}

} // namespace gore