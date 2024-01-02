#pragma once

#include "VulkanIncludes.h"

#include <filesystem>
#include <map>
#include <string>

namespace gore
{

class VulkanDevice;

enum class ShaderStage : uint32_t
{
    Vertex = 1 << 0,
    TessellationControl = 1 << 1,
    TessellationEvaluation = 1 << 2,
    Geometry = 1 << 3,
    Fragment = 1 << 4,
    Compute = 1 << 5,

    Hull = TessellationControl,
    Domain = TessellationEvaluation,
    Pixel = Fragment,

    // TODO: Ray Tracing
    // TODO: Mesh

    AllGraphics = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment,
    All = AllGraphics | Compute,
};
FLAG_ENUM_CLASS(ShaderStage, uint32_t)

class VulkanShader
{
public:
    VulkanShader(VulkanDevice* device, const std::filesystem::path& path, ShaderStage stages);
    ~VulkanShader();

    NON_COPYABLE(VulkanShader);

    [[nodiscard]] VkShaderModule Get(ShaderStage stage) const;
    [[nodiscard]] VulkanDevice* GetDevice() const { return m_Device; }

    [[nodiscard]] VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo(ShaderStage stage, const char* entryPoint) const;

private:
    VulkanDevice* m_Device;

    std::map<ShaderStage, VkShaderModule> m_ShaderModules;
    ShaderStage m_Stages;

    VkShaderModule LoadShader(const std::filesystem::path& path);
};

} // namespace gore
