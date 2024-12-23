#pragma once

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"

struct PerframeData
{
    gore::Matrix4x4 vpMatrix;

    gore::Matrix4x4 directionalLightVPMatrix;
    gore::Vector3 directionalLightColor;
    float directionalLightIntensity;
};