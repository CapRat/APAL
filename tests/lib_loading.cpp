#include "lib_loading.hpp"

#ifdef _WIN32
#include <windows.h>
const char* lastError = nullptr;

library_t LoadLib(const char* libName)
{
    HMODULE lastLib = LoadLibraryA(libName);
    if (lastLib == NULL)
        lastError = "Could not load lib";
    return lastLib;
}

void* LoadFuncRaw(library_t lib, const char* fncName)
{
    FARPROC lastFunc = GetProcAddress(static_cast<HMODULE>(lib), fncName);
    if (lastFunc == NULL)
        lastError = "Could not load function";
    return static_cast<void*>(lastFunc);
}

const char* GetError()
{
    const char* myErr = lastError;
    lastError = NULL;
    return myErr;
}

void UnloadLib(library_t lib)
{
    FreeLibrary(static_cast<HMODULE>(lib));
}
#else
#include <dlfcn.h>
library_t LoadLib(const char* libName)
{
    return dlopen(libName, RTLD_NOW);
}

void* LoadFuncRaw(library_t lib, const char* fncName)
{
    return dlsym(lib, fncName);
}

const char* GetError()
{
    return dlerror();
}

void UnloadLib(library_t lib)
{
    dlclose(lib);
}

#endif //!_WIN32
