#ifndef TYPES_HPP
#define TYPES_HPP
#include <array>
#include <exception>
#include <string>
#include <vector>

#ifdef defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L
#include <string_view>
#else
#include "compatibility/string_view.hpp"
namespace std {
    using namespace nonstd;
    using namespace sv_lite;
}

#endif
namespace XPlug {
struct NotImplementedException : public std::exception {
    const char* what() const noexcept
    {
        return "functionality is not implemented yet.";
    }
};

//std::array<float, 5> x;
//template <size_t size>
//typedef std::array<float,size> audio_data;
}
#endif //! TYPES_HPP
