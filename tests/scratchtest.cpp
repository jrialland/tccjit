#include <catch2/catch_test_macros.hpp>

#include "jit.hpp"

TEST_CASE("Compile and call", "[jit]") {

    jit::Module module(
        "int add(int a, int b, int c) {"
        "    return a + b + c;"
        "}"
    );

    auto add = module.fn<int(int, int, int)>("add");

    REQUIRE(add(1,2,3) == 6);

}