#ifndef LIB_LOADING_HPP
#define LIB_LOADING_HPP
#include <string>
namespace XPlug {
typedef void* library_t;

/**
 * @brief Loads a Library with the given name or path.
 * @param libName name or path to load library from.
 * @return loaded libraryhandle, wich must also be unloaded if not used anymore.
 */
library_t
LoadLib(const char* libName);

/**
 * @brief Loads a functionpointer. The type is void*.
 * @param lib library to load function from.
 * @param fncName functionname to laod.
 * @return void* functionpointer or nullptr if not found.
 */
void*
LoadFuncRaw(library_t lib, const char* fncName);
/**
 * @brief Loads a function with given name from library
 * @tparam T Function signature.
 * @param lib library to load Function from.
 * @param fncName Name of the Function.
 * @return Function with given functionsignature T.
 */
template<typename T>
T
LoadFunc(library_t lib, const char* fncName)
{
  return (T)(LoadFuncRaw(lib, fncName));
}

const char*
GetError();
/**
 * @brief Implements the same as GetError. Just uses the c++ way of handling
 * strings.
 * @return  c++ string
 */
std::string
GetErrorStr();
/**
 * @brief Unloads everything from the loaded lib.
 * @param lib  Library to free or unload.
*/
void
UnloadLib(library_t lib);
}
#endif // ! LIB_LOADING_HPP
