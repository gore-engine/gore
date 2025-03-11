#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

namespace gore::gfx
{
struct GlobalConstantBuffer
{
    Matrix4x4 vpMatrix;

    Matrix4x4 directionalLightVPMatrix;
    Vector3 directionalLightColor;
    float directionalLightIntensity;
};
} // namespace gore::gfx