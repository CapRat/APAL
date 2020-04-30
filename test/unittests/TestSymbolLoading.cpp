#ifndef EXPORT_SYMBOL_LOADING
/**
 * Used for Loading the Shared library and run testcases
 */
#include "GlobalData.hpp"
#include "catch2/catch.hpp"
#include "generated/XPlug_version.h"
#include "lib_loading.hpp"
#ifdef _WIN32
#define SYMBOL_EXPORT_TEST_LIB "ExportLib"
#else
#define SYMBOL_EXPORT_TEST_LIB "./libExportLib.so"
#endif //! _WIN32

library_t __symbolExportLib = nullptr;

library_t getSymbolExportLib()
{
    GetError();
    if (__symbolExportLib == nullptr)
        __symbolExportLib = LoadLib(SYMBOL_EXPORT_TEST_LIB);
    if (__symbolExportLib == nullptr)
        INFO(GetError());
    REQUIRE(__symbolExportLib != nullptr);
    return __symbolExportLib;
}
template <typename T>
T LoadTestFunc(library_t lib, const char* fncname)
{
    GetError();
    T fnc = LoadFunc<T>(lib, fncname);
    if (fnc == nullptr)
        INFO(GetError());
    REQUIRE(fnc != nullptr);
    return fnc;
}

#include <iostream>
/**
 * Test if the loading of symbols work proberly
 */
TEST_CASE("Symbolload checking", "[symbol]")
{

    REQUIRE(getSymbolExportLib() != nullptr);
    auto fnc = LoadFunc<int (*)()>(getSymbolExportLib(), "test_function");

    REQUIRE(fnc != nullptr);
    REQUIRE(fnc() == 2);

    SECTION("MODULE LOADING TEST FUNCTION LOADING")
    {
        auto fnc2 = LoadFunc<VERSION (*)()>(getSymbolExportLib(), "XPlugGetVersion");
        REQUIRE(fnc2 != nullptr);
        REQUIRE(fnc2().MAJOR == XPlug_VERSION.MAJOR);
    }
}

#else
/** 
 * Used for building an shared library.
 */
#include "exportlib_export.h"
//#include "plugins/ladspa.h"
#include "GlobalData.hpp"
extern "C" {

int EXPORTLIB_EXPORT test_function()
{
    return 2;
}
}

#endif
