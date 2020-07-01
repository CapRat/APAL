#include <tools/StringTools.hpp>

/*
*Get File Name from a Path with or without extension
*/
std::string XPlug::getFileName(std::string filePath, bool withExtension, char seperator)
{
    while (filePath.find("\\") != std::string::npos)
        filePath.replace(filePath.find("\\"), 1, "/");
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    std::size_t sepPos = filePath.rfind(seperator);
    if (sepPos != std::string::npos)
    {
        return filePath.substr(sepPos + 1, (!withExtension && dotPos != std::string::npos ? dotPos : filePath.size()) - sepPos - 1);
    }
    return "";
}