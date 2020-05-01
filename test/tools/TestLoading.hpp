#ifndef TEST_LOADING_HPP
#define TEST_LOADING_HPP
#include "catch2/catch.hpp"
#include "lib_loading.hpp"
/**
 * @brief LoadTestLib Loads the given lib, but it requires the Library to load correctly. Otherwhise the Test will be abborted
 * @return the given lib
 */
inline library_t LoadTestLib(const char* libname)
{
    GetError();
    library_t symbolExportLib = LoadLib(libname);
    if (symbolExportLib == nullptr)
        UNSCOPED_INFO(GetError());
    REQUIRE(symbolExportLib != nullptr);
    return symbolExportLib;
}

template <typename T>
/**
 * @brief LoadTestFunc Loads a Function from a library, if not possibile Error is noted and Test is abborted
 * @param lib library to load symbol from
 * @param fncname Functionname to load
 * @return Castet function, which should be loaded.
 */
T LoadTestFunc(library_t lib, const char* fncname)
{
    GetError();
    T fnc = LoadFunc<T>(lib, fncname);
    if (fnc == nullptr)
        UNSCOPED_INFO(GetError());
    REQUIRE(fnc != nullptr);
    return fnc;
}
#endif //! TEST_LOADING_HPP
