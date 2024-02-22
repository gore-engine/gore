#pragma once

#include "Math/Matrix4x4.h"

namespace gore::gfx
{
struct GlobalConstantBuffer
{
    Matrix4x4 vpMatrix;
};
} // namespace gore::gfx