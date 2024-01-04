#include "Prefix.h"

#include "FileSystem.h"

#include "Core/App.h"

#include <fstream>

namespace gore
{

std::filesystem::path FileSystem::s_ExecutablePath;
std::filesystem::path FileSystem::s_ResourceFolder;

std::filesystem::path FileSystem::GetExecutablePath()
{
    if (s_ExecutablePath.empty())
    {
        s_ExecutablePath = App::Get()->m_ExecutablePath;
    }

    return s_ExecutablePath;
}

#if PLATFORM_MACOS
std::string GetMacOSAppBundleResourcePath();
#endif

std::filesystem::path FileSystem::GetResourceFolder()
{
    if (!s_ResourceFolder.empty())
    {
        return s_ResourceFolder;
    }

#if PLATFORM_MACOS
    s_ResourceFolder = GetMacOSAppBundleResourcePath();
#else
    s_ResourceFolder = s_ExecutablePath / "Resources";
#endif
    return s_ResourceFolder;
}

std::vector<char> FileSystem::ReadAllBinary(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return {};
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    file.close();

    return buffer;
}

}