#include "TestLoading.hpp"

library_t LoadTestLib(const char* libname)
{
    GetError();
    library_t symbolExportLib = LoadLib(libname);
    if (symbolExportLib == nullptr)
        UNSCOPED_INFO(GetError());
    REQUIRE(symbolExportLib != nullptr);
    return symbolExportLib;
}
