
/**
 * Used for Loading the Shared library and run testcases
 **/
#include "TestLoading.hpp"
#ifdef _WIN32
#define SYMBOL_EXPORT_TEST_LIB "test_components/simple_symbol_export"
#else
#define SYMBOL_EXPORT_TEST_LIB "test_components/libsimple_symbol_export.so"
#endif //! _WIN32

#include <iostream>
/**
 * Test if the loading of symbols work proberly
 */
TEST_CASE("Symbolload checking", "[symbol]")
{

    auto lib = LoadTestLib(SYMBOL_EXPORT_TEST_LIB);
    auto test_function = LoadTestFunc<int (*)()>(lib, "test_function");
    REQUIRE(test_function() == 2);

    SECTION("MODULE LOADING TEST FUNCTION LOADING")
    {
        auto static_test_function = LoadTestFunc<int (*)(int)>(lib, "static_test_function");
        REQUIRE(static_test_function(2) == 2 + 5);
        //Load Hidden function
        auto static_test_function2 = LoadTestFunc<int (*)(int)>(lib, "static_test_function2");
        REQUIRE(static_test_function2(2) == 2 + 5);
    }
}
