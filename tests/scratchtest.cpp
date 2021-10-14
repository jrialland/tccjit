#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "tccjit.hpp"
#include <iostream>
#include <map>

using namespace std;



TEST_CASE("Compile and call", "[jit]") {
    
    // compile some code
    
    jit::Module * pm1 = new jit::Module(
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

        auto add2 = pm1->fn<int(int, int)>("add2");
        auto add3 = m2.fn<int(int, int, int)>("add3");

        REQUIRE(add2(1,2) == 3);
        REQUIRE(add3(1,2,3) == 6);
    }

    delete pm1;

    // "malloc(): invalid size (unsorted)"
    void *p = malloc(400 * 1000);
    free(p);

}
