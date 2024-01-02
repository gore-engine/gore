#pragma once

// How to write a Catch2 Test
// 1. include our fancy TestPrefix.h
// 2. Add ENABLE_TEST Macro
// 3. check https://github.com/catchorg/Catch2 for best practice
#ifdef ENABLE_TEST
#include <catch2/catch_test_macros.hpp>
#endif