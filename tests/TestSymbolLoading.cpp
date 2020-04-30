#ifndef EXPORT_SYMBOL_LOADING
/**
 * Used for Loading the Shared library and run testcases
 */
#include "catch2/catch.hpp"
#include "lib_loading.hpp"
#include "GlobalData.hpp"
#include "generated/XPlug_version.h"

#define SYMBOL_EXPORT_TEST_LIB "ExportLib"

library_t __symbolExportLib = nullptr;

library_t getSymbolExportLib() {
	if(__symbolExportLib ==nullptr)
		__symbolExportLib =LoadLib(SYMBOL_EXPORT_TEST_LIB);
	return __symbolExportLib;
}


/**
 * Test if the loading of symbols work proberly
 */
TEST_CASE("Symbolload checking","[symbol]") {
	auto fnc = LoadFunc<int (*)()>(getSymbolExportLib(), "test_function");
	REQUIRE(fnc != nullptr);
	REQUIRE(fnc() == 2);

	SECTION("MODULE LOADING TEST FUNCTION LOADING") {
		auto fnc2 = LoadFunc<VERSION(*)()>(getSymbolExportLib(), "XPlugGetVersion");
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


	int EXPORTLIB_EXPORT test_function() {
		return 2;
	}
}

#endif