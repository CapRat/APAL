
/**
 * Used for Loading the Shared library and run testcases
 **/
#include "tools/LibLoading.hpp"
#include "UnitTools.hpp"
using namespace XPlug;
#include "catch2/catch.hpp"
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

    auto lib = LoadLib(SYMBOL_EXPORT_TEST_LIB);
    REQUIRE_MESSAGE(lib != nullptr, GetErrorStr());
    
    auto test_function = LoadFunc<int (*)()>(lib, "test_function");
    REQUIRE_MESSAGE(test_function != nullptr, GetErrorStr());
    REQUIRE(test_function() == 2);

    SECTION("MODULE LOADING TEST FUNCTION LOADING")
    {
        auto static_test_function = LoadFunc<int (*)(int)>(lib, "static_test_function");
        REQUIRE_MESSAGE(static_test_function != nullptr, GetErrorStr());
        REQUIRE(static_test_function(2) == 2 + 5);
        //Load Hidden function
        auto static_test_function2 = LoadFunc<int (*)(int)>(lib, "static_test_function2");
        REQUIRE_MESSAGE(static_test_function2 != nullptr, GetErrorStr());
        REQUIRE(static_test_function2(2) == 2 + 5);
    }
}
