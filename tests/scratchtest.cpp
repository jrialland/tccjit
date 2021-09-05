#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "tccjit.hpp"
#include <iostream>
using namespace std;

TEST_CASE("Compile and call", "[jit]") {
    jit::Module m1(
        "int add2(int a, int b) {"
        "    return a + b;"
        "}"
    );
    jit::Module m2(
        "int add3(int a, int b, int c) {"
        "    return a + b + c;"
        "}"
    );
    for(int i=0; i < 100; i++) {

        auto add2 = m1.fn<int(int, int)>("add2");
        auto add3 = m2.fn<int(int, int, int)>("add3");

        REQUIRE(add2(1,2) == 3);
        REQUIRE(add3(1,2,3) == 6);
    }
}
