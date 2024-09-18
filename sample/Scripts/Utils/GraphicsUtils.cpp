#include "GraphicsUtils.h"

#include "FileSystem/FileSystem.h"

namespace sample::utils
{
std::vector<char> LoadShaderBytecode(const std::string& name, const ShaderStage& stage, const std::string& entryPoint)
{
    using namespace gore;
    
    static const std::filesystem::path kShaderSourceFolder = FileSystem::GetResourceFolder() / "Shaders";

    auto getShaderFile = [&name](ShaderStage stage) -> std::filesystem::path
    {
        std::filesystem::path path(name);
        auto shaderPath = kShaderSourceFolder / path.parent_path() / path.filename().stem();
        shaderPath += std::string(".") + (stage == ShaderStage::Vertex ? "vert" : "frag") + ".spv";
        return shaderPath;
    };

    std::filesystem::path shaderPath = getShaderFile(stage);

    return FileSystem::ReadAllBinary(shaderPath);
}
} // namespace sample::utils