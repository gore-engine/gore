#pragma once

// How to write a Catch2 Test
// 1. Create a file suffixed with Test
// 2. include our fancy TestPrefix.h
// 3. Add ENABLE_TEST Macro
// 4. check https://github.com/catchorg/Catch2 for best practice

// Exmaple:

// ExmapleTest.cpp

// #include "TestPrefix.h"
// #ifdef ENABLE_TEST
// static int Factorial( int number ) {
//    return number <= 1 ? number : Factorial( number - 1 ) * number;  // fail
// }
// TEST_CASE( "Factorial of 0 is 1 (fail)", "[single-file]" ) {
//     REQUIRE( Factorial(0) == 1 );
// }
// TEST_CASE( "Factorials of 1 and higher are computed (pass)", "[single-file]" ) {
//     REQUIRE( Factorial(1) == 1 );
//     REQUIRE( Factorial(2) == 2 );
//     REQUIRE( Factorial(3) == 6 );
//     REQUIRE( Factorial(10) == 3628800 );
// }
// #endif

#ifdef ENABLE_TEST
#include <catch2/catch_test_macros.hpp>
#endif