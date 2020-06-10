#include "tools/LibLoading.hpp"

namespace XPlug {
    std::string GetErrorStr()
    {
        const char* errStr = GetError();
        return errStr==nullptr? std::string():errStr;
    }
}
#ifdef _WIN32
#include <windows.h>
namespace XPlug {
    char* lastError = nullptr;
    char* lastErrorReturnCopy = nullptr;
    library_t LoadLib(const char* libName)
    {
        HMODULE lastLib = LoadLibraryA(libName);
        if (lastLib == NULL) {
            static const char NO_LIB_LOAD_MSG[] = "Could not load library: ";
            free((void*)lastError);
            lastError= (char*)malloc(sizeof(NO_LIB_LOAD_MSG) + strlen(libName));
            strcpy(lastError, NO_LIB_LOAD_MSG);
            strcat(lastError, libName);
        }
        return lastLib;
    }

    void* LoadFuncRaw(library_t lib, const char* fncName)
    {
        FARPROC lastFunc = GetProcAddress(static_cast<HMODULE>(lib), fncName);
        if (lastFunc == NULL) {
            static const char NO_FNC_LOAD_MSG[] = "Could not load function: ";
            free((void*)lastError);
            lastError = (char*)malloc(sizeof(NO_FNC_LOAD_MSG) + strlen(fncName));
            strcpy(lastError, NO_FNC_LOAD_MSG);
            strcat(lastError, fncName);
        }
        return reinterpret_cast<void*>(lastFunc);
    }

    const char* GetError()
    {
        free(lastErrorReturnCopy);
        lastErrorReturnCopy = lastError;
        lastError = NULL;
        return lastErrorReturnCopy;
    }

    void UnloadLib(library_t lib)
    {
        free(lastError);
        free(lastErrorReturnCopy);
        FreeLibrary(static_cast<HMODULE>(lib));
    }
}
#else
#include <dlfcn.h>
namespace XPlug {
    library_t LoadLib(const char* libName)
    {
        return dlopen(libName, RTLD_LAZY);
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
}
#endif //!_WIN32
