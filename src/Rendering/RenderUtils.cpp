#include "Prefix.h"

#include "RenderUtils.h"

#include <sstream>
#include <iomanip>

namespace gore
{

std::string VendorIDToString(VendorID vendorID)
{
    struct KnownVendors
    {
        VendorID vendorID;
        const char* name;
    };

    // clang-format off
    static const KnownVendors s_KnownVendors[] =
    {
        { VendorID::None,         "NONE" },
        { VendorID::Intel,        "Intel" },
        { VendorID::ATI,          "ATI" },
        { VendorID::NVIDIA,       "NVIDIA" },
        { VendorID::Microsoft,    "Microsoft" },
        { VendorID::Qualcomm,     "Qualcomm" },
        { VendorID::ARM,          "ARM" },
        { VendorID::ImgTech,      "Imagination Technologies" },
        { VendorID::Samsung,      "Samsung" },
        { VendorID::Apple,        "Apple" },
        { VendorID::MooreThreads, "Moore Threads"},
    };
    // clang-format on

    static int kKnownVendorsSize = sizeof(s_KnownVendors) / sizeof(s_KnownVendors[0]);

    for (int i = 0; i < kKnownVendorsSize; ++i)
    {
        if (s_KnownVendors[i].vendorID == vendorID)
        {
            return s_KnownVendors[i].name;
        }
    }

    std::stringstream ss;
    ss << "Unknown (ID=" << std::hex << static_cast<uint32_t>(vendorID) << ")";
    return ss.str();
}

}