#ifndef STRING_TOOLS_HPP
#define STRING_TOOLS_HPP
#include <string>
namespace XPlug {
    /*
     *Get File Name from a Path with or without extension
     */
    std::string getFileName(std::string filePath, bool withExtension = true, char seperator = '/');
}
#endif //! STRING_TOOLS_HPP