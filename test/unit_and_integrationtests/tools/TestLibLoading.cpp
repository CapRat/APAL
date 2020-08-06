/**
 * This file contains multiple Sources, which are controlled by preprocessor define. 
 * This other Sources are needed for the Unittest. If no BUILD_TEST_LIB preprocessor
 * directive is defined, the normal Unittest is build.
 * if its defined 2 libraries can be build. To build the static library define 
 * BUILD_STATIC_EXAMPLE and to build  tge simple symbol export define BUILD_SIMPLE_SYMBOL_EXPORT.
 */


#ifndef BUILD_TEST_LIB
/**
 * Used for Loading the Shared library and run testcases
 **/
#include "tools/LibLoading.hpp"
#include "CatchTools.hpp"
using namespace XPlug;
#ifdef _WIN32
#define SYMBOL_EXPORT_TEST_LIB "simple_symbol_export"
#else
#define SYMBOL_EXPORT_TEST_LIB "./libsimple_symbol_export.so"
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
#else
/**Static testlibrary, to link this static function in an shared library and check if function is inside of it**/

extern "C" {
    int static_test_function(int x);
};

#ifdef BUILD_STATIC_EXAMPLE
//Static Example Content:
//#include "test_component.hpp"

int static_test_function(int x)
{
    return 5 + x;
}

extern "C" {
    int static_test_function2(int x)
    {
        return 5 + x;
    }
}
#endif

#ifdef BUILD_SIMPLE_SYMBOL_EXPORT

//SimpleSymbolExport Content:
#include "simple_symbol_export_export.h"
//#include "test_component.hpp"

extern "C" {

    int SIMPLE_SYMBOL_EXPORT_EXPORT test_function()
    {
        return 2;
    }
}

#endif
#endif
