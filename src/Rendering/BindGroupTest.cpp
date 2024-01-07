#include "Test/TestPrefix.h"

#if ENABLE_TEST
    #include "BindGroup.h"
    #include "BindLayout.h"

namespace gore
{
namespace test
{
TEST_CASE("BindGroupDesc struct tests", "[BindGroupDesc]")
{
    SECTION("Default values")
    {
        BindGroupDesc bg;
        REQUIRE(std::strcmp(bg.debugName, "Noname BindGroupDesc") == 0);
        REQUIRE(bg.bindLayout == nullptr);
        REQUIRE(bg.textures.empty());
        REQUIRE(bg.buffers.empty());
    }

    SECTION("Change values")
    {
        BindGroupDesc bg;
        bg.debugName = "Test BindGroupDesc";
        BindLayout bl;
        bg.bindLayout = &bl;
        TextureHandle th;
        bg.textures.push_back(th);
        BufferHandle bh;
        bg.buffers.push_back({bh, 0});

        REQUIRE(std::strcmp(bg.debugName, "Test BindGroupDesc") == 0);
        REQUIRE(bg.bindLayout == &bl);
        REQUIRE(bg.textures.size() == 1);
        REQUIRE(bg.buffers.size() == 1);
    }
}
} // namespace test
} // namespace gore
#endif