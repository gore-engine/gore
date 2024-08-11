#include "RpsSytem.h"

namespace gore::gfx
{
std::unique_ptr<RpsSytem> InitializeRPSSystem(const RPSSytemCreateInfo& createInfo)
{
    auto rpsSystem = std::make_unique<RpsSytem>();
#if RPS_VK_RUNTIME
#endif
    return rpsSystem;
}
} // namespace gore::gfx