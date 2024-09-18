#pragma once

#include <vector>
#include <string>

#include "Rendering/GraphicsResourcePrefix.h"

namespace sample::utils
{
std::vector<char> LoadShaderBytecode(const std::string& name, const ShaderStage& stage, const std::string& entryPoint);
} // namespace sample::utils 