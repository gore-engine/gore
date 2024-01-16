#pragma once

#include <string>

namespace gore::gfx
{

enum class VendorID : uint32_t
{
    None         = 0x0000,
    Intel        = 0x8086,
    ATI          = 0x1002,
    NVIDIA       = 0x10de,
    Microsoft    = 0x1414,
    Qualcomm     = 0x5143,
    ARM          = 0x13b5,
    ImgTech      = 0x1010,
    Samsung      = 0x144d,
    Apple        = 0x106b,
    MooreThreads = 0x1ed5,
};

std::string VendorIDToString(VendorID vendorID);

} // namespace gore::gfx
