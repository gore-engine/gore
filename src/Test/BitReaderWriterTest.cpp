#include "TestPrefix.h"
#ifdef ENABLE_TEST

#include "../Utilities/BitReader.h"
#include "../Utilities/BitWriter.h"

using namespace gore;

TEST_CASE("BitReaderWriterTest", "[single-file]")
{
    BitReader reader(nullptr, 0);
    REQUIRE(reader.GetBitsCount() == 0);
    
    BitWriter writer(0, false);
    REQUIRE(writer.GetBitsCount() == 0);
}

#endif // ENABLE_TEST