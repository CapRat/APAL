#ifndef TYPES_HPP
#define TYPES_HPP
#include <array>
#include <exception>
#include <string>
#include <vector>

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L
#include <string_view>
#else
#include "compatibility/string_view.hpp"
namespace std {
using namespace nonstd;
using namespace sv_lite;
}

#endif
namespace XPlug {
/**
 * @brief NotImplementedException, to be used when something in this project is
 * not implemented yet. Currently not used that much, but in the opensource
 * cycle it should be used more.
 */
struct NotImplementedException : public std::exception
{
  const char* what() const noexcept
  {
    return "functionality is not implemented yet.";
  }
};
}
#endif //! TYPES_HPP
