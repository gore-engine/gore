#include "Prefix.h"

#include "VulkanShader.h"
#include "VulkanDevice.h"

#include "Core/Log.h"
#include "FileSystem/FileSystem.h"

#include <string>

namespace gore
{

static const std::filesystem::path kShaderSourceFolder = FileSystem::GetResourceFolder() / "Shaders";

static std::string GetOpenGLShaderStageName(ShaderStage stage);

VulkanShader::VulkanShader(VulkanDevice* device, const std::filesystem::path& path, ShaderStage stages) :
    m_Device(device),
    m_ShaderModules(),
    m_Stages(stages)
{
    auto getShaderFile = [&path](ShaderStage stage) -> std::filesystem::path
    {
        auto shaderPath = kShaderSourceFolder / path.parent_path() / path.filename().stem();
        shaderPath += "." + GetOpenGLShaderStageName(stage) + ".spv";
        return shaderPath;
    };

    // TODO: Magic number here!
    for (uint32_t i = 0; i < 6; ++i)
    {
        auto stage = static_cast<ShaderStage>(1 << i);
        if (HasFlag(stages, stage))
        {
            auto shaderPath        = getShaderFile(stage);
            m_ShaderModules[stage] = LoadShader(shaderPath);
        }
    }
}

VulkanShader::~VulkanShader()
{
    for (auto& [stage, shaderModule] : m_ShaderModules)
    {
        m_Device->API.vkDestroyShaderModule(m_Device->Get(), shaderModule, VK_NULL_HANDLE);
    }
}

VkShaderModule VulkanShader::Get(ShaderStage stage) const
{
    if (HasFlag(m_Stages, stage))
    {
        return m_ShaderModules.at(stage);
    }

    return VK_NULL_HANDLE;
}

VkShaderModule VulkanShader::LoadShader(const std::filesystem::path& path)
{
    std::vector<char> shaderBinary = FileSystem::ReadAllBinary(path);
    if (shaderBinary.empty())
    {
        LOG_STREAM(ERROR) << "Failed to load shader: " << path << std::endl;
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderBinary.size(),
        .pCode    = reinterpret_cast<const uint32_t*>(shaderBinary.data()),
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    VkResult res = m_Device->API.vkCreateShaderModule(m_Device->Get(), &createInfo, VK_NULL_HANDLE, &shaderModule);
    VK_CHECK_RESULT(res);

    return shaderModule;
}

VkPipelineShaderStageCreateInfo VulkanShader::GetShaderStageCreateInfo(ShaderStage stage, const char* entryPoint) const
{
    VkShaderModule shaderModule = Get(stage);
    if (shaderModule == VK_NULL_HANDLE)
    {
        return {};
    }

    VkPipelineShaderStageCreateInfo createInfo{
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = static_cast<VkShaderStageFlagBits>(stage),
        .module = shaderModule,
        .pName  = entryPoint,
    };

    return createInfo;
}

std::string GetOpenGLShaderStageName(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return "vert";
        case ShaderStage::TessellationControl:
            return "tesc";
        case ShaderStage::TessellationEvaluation:
            return "tese";
        case ShaderStage::Geometry:
            return "geom";
        case ShaderStage::Fragment:
            return "frag";
        case ShaderStage::Compute:
            return "comp";
        default:
            return "unknown";
    }
}

} // namespace gore