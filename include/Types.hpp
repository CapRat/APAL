#ifndef TYPES_HPP
#define TYPES_HPP
#include <string>
#include <array>
#include <vector>
#include <exception>
namespace XPlug {
    struct NotImplementedException : public std::exception
    {
        const char* what() const throw ()
        {
            return "functionality is not implemented yet.";
        }
    };

    //std::array<float, 5> x;
    //template <size_t size>
    //typedef std::array<float,size> audio_data;
}
#endif //! TYPES_HPP