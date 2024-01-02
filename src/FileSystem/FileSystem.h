#pragma once

#include "Export.h"

#include <vector>
#include <filesystem>

namespace gore
{

ENGINE_CLASS(FileSystem)
{
public:
    static std::filesystem::path GetExecutablePath();
    static std::filesystem::path GetResourceFolder();
    static std::vector<char> ReadAllBinary(const std::filesystem::path& path);

private:
    static std::filesystem::path s_ExecutablePath;
    static std::filesystem::path s_ResourceFolder;
};

} // namespace gore
