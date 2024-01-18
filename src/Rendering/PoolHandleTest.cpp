#include "Test/TestPrefix.h"
#include "Pool.h"

#if ENABLE_TEST
namespace gore {
namespace test {

struct TestObject {
    int value;
    bool operator==(const TestObject& other) const {
        return value == other.value;
    }
};

TEST_CASE("Pool class tests", "[Pool]") {
    SECTION("Create and destroy") {
        Pool<TestObject, TestObject> pool;
        TestObject objDesc{1};
        TestObject obj{2};
        auto handle = pool.create(std::move(objDesc), std::move(obj));
        REQUIRE(pool.num() == 1);
        pool.destroy(handle);
        REQUIRE(pool.num() == 0);
    }

    SECTION("Get") {
        Pool<TestObject, TestObject> pool;
        TestObject objDesc{1};
        TestObject obj{2};
        auto handle = pool.create(std::move(objDesc), std::move(obj));
        REQUIRE(pool.getObjectPtr(handle)->value == 2);
    }

    SECTION("Find object") {
        Pool<TestObject, TestObject> pool;
        TestObject objDesc{1};
        TestObject obj{2};
        auto handle = pool.create(std::move(objDesc), std::move(obj));
        auto foundHandle = pool.getObjectHandle(&obj);
        REQUIRE(foundHandle == handle);
    }

    SECTION("Clear") {
        Pool<TestObject, TestObject> pool;
        TestObject objDesc{1};
        TestObject obj{2};
        pool.create(std::move(objDesc), std::move(obj));
        pool.clear();
        REQUIRE(pool.num() == 0);
    }
}

}  // namespace test
}  // namespace gore
#endif