#ifndef LIB_LOADING_HPP
#define LIB_LOADING_HPP
#include <string>
namespace XPlug {
    typedef void* library_t;

    library_t LoadLib(const char* libName);

    void* LoadFuncRaw(library_t lib, const char* fncName);

    template <typename T>
    T LoadFunc(library_t lib, const char* fncName)
    {
        return (T)(LoadFuncRaw(lib, fncName));
    }

    const char* GetError();
    /**
     * @brief Implements the same as GetError. Just uses the c++ way of handling strings.
     * @return  c++ string
    */
    std::string GetErrorStr();
    void UnloadLib(library_t lib);
}
#endif // ! LIB_LOADING_HPP
