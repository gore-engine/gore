#include "TestPrefix.h"
#ifdef ENABLE_TEST

#include "../Utilities/BitReader.h"
#include "../Utilities/BitWriter.h"

namespace gore
{
class BitWriterTest
{
public:
    BitWriterTest() :
        buffer(16, 0),
        bitWriter(buffer.size() * 8, true)
    {
    }

    std::vector<uint8_t> buffer;
    BitWriter bitWriter;
};

TEST_CASE_METHOD(BitWriterTest, "BitWriter Initialization", "[BitWriter]")
{
    REQUIRE(bitWriter.GetBitsCount() == 128);
    REQUIRE(bitWriter.GetBitsWritten() == 0);
    REQUIRE(bitWriter.GetBitsRemaining() == 128);
}

TEST_CASE_METHOD(BitWriterTest, "BitWriter WriteUInt8", "[BitWriter]")
{
    bitWriter.WriteUInt8(0xFF);
    REQUIRE(bitWriter.GetBitsWritten() == 8);
    REQUIRE(bitWriter.GetBitsRemaining() == 120);
}

TEST_CASE_METHOD(BitWriterTest, "BitWriter WriteUInt16", "[BitWriter]")
{
    bitWriter.WriteUInt16(0xFFFF);
    REQUIRE(bitWriter.GetBitsWritten() == 16);
    REQUIRE(bitWriter.GetBitsRemaining() == 112);
}

TEST_CASE_METHOD(BitWriterTest, "BitWriter WriteUInt32", "[BitWriter]")
{
    bitWriter.WriteUInt32(0xFFFFFFFF);
    REQUIRE(bitWriter.GetBitsWritten() == 32);
    REQUIRE(bitWriter.GetBitsRemaining() == 96);
}

TEST_CASE_METHOD(BitWriterTest, "BitWriter AllowResize", "[BitWriter]")
{
    REQUIRE(bitWriter.IsAllowResize() == true);
}

class BitReaderTest
{
public:
    BitReaderTest() :
        data(10, 0),
        bitReader(data.data(), data.size())
    {
        data[0] = 0x10; // 00010000
        data[1] = 0x20; // 00100000
        data[2] = 0x40; // 01000000
        data[3] = 0x80; // 10000000
    }

    std::vector<uint8_t> data;
    BitReader bitReader;
};

TEST_CASE_METHOD(BitReaderTest, "BitReader Initialization", "[BitReader]")
{
    REQUIRE(bitReader.GetBitsCount() == 80);
    REQUIRE(bitReader.GetBitsRead() == 0);
    REQUIRE(bitReader.GetBitsRemaining() == 80);
}

TEST_CASE_METHOD(BitReaderTest, "BitReader ReadUInt8", "[BitReader]")
{
    REQUIRE(bitReader.ReadUInt8() == 16);
    REQUIRE(bitReader.GetBitsRead() == 8);
    REQUIRE(bitReader.GetBitsRemaining() == 72);
}

TEST_CASE_METHOD(BitReaderTest, "BitReader ReadUInt16", "[BitReader]")
{
    REQUIRE(bitReader.ReadUInt16() == 0x2010);
    REQUIRE(bitReader.GetBitsRead() == 16);
    REQUIRE(bitReader.GetBitsRemaining() == 64);
}

TEST_CASE_METHOD(BitReaderTest, "BitReader ReadUInt32", "[BitReader]")
{
    REQUIRE(bitReader.ReadUInt32() == 0x80402010);
    REQUIRE(bitReader.GetBitsRead() == 32);
    REQUIRE(bitReader.GetBitsRemaining() == 48);
}
} // namespace gore
#endif // ENABLE_TEST