#include "Prefix.h"

#include "Core/Log.h"
#include "RenderSystem.h"

#include <iostream>
#include <vector>

#if PLATFORM_ANDROID || PLATFORM_LINUX || PLATFORM_MACOS
    #include <dlfcn.h>
#elif PLATFORM_WIN
    #include <windows.h>
#endif

namespace gore
{

static RenderSystem* g_RenderSystem = nullptr;

#if PLATFORM_ANDROID
const char* const kVulkanLibraryName = "libvulkan.so";
#elif PLATFORM_WIN
const char* const kVulkanLibraryName = "vulkan-1.dll";
#elif PLATFORM_LINUX
const char* const kVulkanLibraryName = "libvulkan.so.1";
#elif PLATFORM_MACOS
const char* const kVulkanLibraryName = "libvulkan.1.dylib";
#else
    #error Unsupported Platform
#endif

#if PLATFORM_WIN
typedef HMODULE LibraryHandle;
#elif PLATFORM_ANDROID || PLATFORM_LINUX || PLATFORM_MACOS
typedef void* LibraryHandle;
#endif

static LibraryHandle LoadDynamicLibrary(const char* name);
static void UnloadDynamicLibrary(LibraryHandle lib);
static void* LoadSymbol(LibraryHandle lib, const char* func);

static LibraryHandle s_VulkanLibraryHandle = nullptr;

RenderSystem::RenderSystem(gore::App* app) :
    System(app),
    m_Instance(VK_NULL_HANDLE),
    m_PhysicalDevice(VK_NULL_HANDLE),
    m_Device(VK_NULL_HANDLE)
{
    g_RenderSystem = this;
}

RenderSystem::~RenderSystem()
{
    g_RenderSystem = nullptr;
}

void RenderSystem::Initialize()
{
    s_VulkanLibraryHandle = LoadDynamicLibrary(kVulkanLibraryName);
    if (!s_VulkanLibraryHandle)
    {
        LOG(FATAL, "Failed to load Vulkan Library\n");
        return;
    }
    LOG(INFO, "Loaded Vulkan Library <%s>\n", kVulkanLibraryName);


}

void RenderSystem::Update()
{
}

void RenderSystem::Shutdown()
{


    if (s_VulkanLibraryHandle)
    {
        UnloadDynamicLibrary(s_VulkanLibraryHandle);
        s_VulkanLibraryHandle = nullptr;
    }
}

void RenderSystem::OnResize(Window* window, int width, int height)
{
}

#if PLATFORM_WIN

LibraryHandle LoadDynamicLibrary(const char* name)
{
    return ::LoadLibraryA(name);
}
void UnloadDynamicLibrary(LibraryHandle lib)
{
    ::FreeLibrary(lib);
}
void* LoadSymbol(LibraryHandle lib, const char* func)
{
    return reinterpret_cast<void*>(::GetProcAddress(lib, func));
}

#elif PLATFORM_ANDROID || PLATFORM_LINUX || PLATFORM_MACOS

LibraryHandle LoadDynamicLibrary(const char* name)
{
    return dlopen(name, RTLD_LOCAL | RTLD_NOW);
}
void UnloadDynamicLibrary(LibraryHandle lib)
{
    dlclose(lib);
}
void* LoadSymbol(LibraryHandle lib, const char* func)
{
    return dlsym(lib, func);
}

#endif

} // namespace gore
