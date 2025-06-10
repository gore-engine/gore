#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"

namespace gore::gfx
{
struct InstanceData
{
    gore::Matrix4x4 preModelMatrix; // Pre-model matrix for the instance
    gore::Matrix4x4 modelMatrix;    // Model matrix for the instance
};
} // namespace gore::gfx