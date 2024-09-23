#pragma once

#include "Rendering/GraphicsFormat.h"

#include <vector>

namespace gore
{
struct RenderPassDesc final
{
    std::vector<GraphicsFormat> colorFormats = {};
    GraphicsFormat depthFormat               = GraphicsFormat::Undefined;
    GraphicsFormat stencilFormat             = GraphicsFormat::Undefined;
};
} // namespace gore