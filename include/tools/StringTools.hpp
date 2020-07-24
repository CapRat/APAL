#ifndef STRING_TOOLS_HPP
#define STRING_TOOLS_HPP
#include <string>
namespace XPlug {
    /**
     * @brief Replaces all occurences of an itemToReplace in the given strToChange
     * @param strToChange string to replaces stuff in.
     * @param itemToReplace  String value, which should be replaces
     * @param substitute  Text to replace itemToReplace with
     */
    std::string replaceInString(std::string strToChange, const std::string itemToReplace, const std::string substitute);


    /**
     * @brief Gets the filename from a path with or without extension
     * When no seperator is found, the whole filePath is returned.
     * @param filePath filepath, to retreive filename from
     * @param withExtension return the fileextension with the filename.
     * @param seperator Seperator for the given fielpath. Defaults to /.
     * @return 
    */
    std::string getFileName(std::string filePath, bool withExtension = true, char seperator = '/');
}
#endif //! STRING_TOOLS_HPP