#include "Test/TestPrefix.h"

#include "DummyVulkan.h"
#include "RenderContext.h"

#include "FileSystem/FileSystem.h"

#if ENABLE_TEST
namespace gore
{
namespace test
{

TEST_CASE("RenderContext can create Vulkan Resource", "[RenderContext]")
{
    SECTION("Create ShaderModule Handle")
    {
        DummyVulkan dummyVulkan;
        dummyVulkan.Initialize();

        dummyVulkan.Shutdown();
    }
}

} // namespace test
} // namespace gore
#endif