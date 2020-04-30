#ifndef LIB_LOADING_HPP
#define LIB_LOADING_HPP

typedef void* library_t;

library_t LoadLib(const char* libName);

void* LoadFuncRaw(library_t lib, const char* fncName);

template <typename T>
T LoadFunc(library_t lib, const char* fncName)
{
    return (T)(LoadFuncRaw(lib, fncName));
}

const char* GetError();
void UnloadLib(library_t lib);

#endif // ! LIB_LOADING_HPP
