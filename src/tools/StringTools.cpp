#include <tools/StringTools.hpp>

std::string
XPlug::replaceInString(std::string strToChange,
                       const std::string itemToReplace,
                       const std::string substitute)
{
  while (strToChange.find(itemToReplace) != std::string::npos)
    strToChange.replace(strToChange.find(itemToReplace), 1, substitute);
  return strToChange;
}

/*
 *Get File Name from a Path with or without extension
 */
std::string
XPlug::getFileName(std::string filePath, bool withExtension, char seperator)
{
  // Get last dot position
  std::size_t dotPos = filePath.rfind('.');
  std::size_t sepPos = filePath.rfind(seperator);
  if (sepPos != std::string::npos) {
    return filePath.substr(sepPos + 1,
                           (!withExtension && dotPos != std::string::npos
                              ? dotPos
                              : filePath.size()) -
                             sepPos - 1);
  }
  return filePath;
}