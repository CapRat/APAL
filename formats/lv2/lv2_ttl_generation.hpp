#ifndef LV2_TTL_GENERATION_H
#define LV2_TTL_GENERATION_H
#include <string>
#include <interfaces/IPlugin.hpp>
#include <vector>
#ifdef __cplusplus
//extern "C" {
#endif 
/**
 * @brief Type, which means that it can be casted to IPlugin*.
*/
typedef void* CIPluginPtr;
struct TTLPluginInfo {
    std::string ttlFileName = ""; // relative filepath(relative in the Bundle) of ttl file with name and extension.
    std::string binFileName = ""; // relative filepath(relative in the Bundle) of binary file with name and extension.
    XPlug::IPlugin* plugPtr = nullptr; // Pointer to IPlugin
};


/**
 * @brief Function to get ttl file content from an Plugin.
 * @param pluginPtr pointer to an IPlugin class. IPlugin is not c compatible, so it must be casted to void*
 * @param ttlContent String return value.
*/
extern "C" std::string getTTLFromPlugin(XPlug::IPlugin * pluginPtr);

/**
 * @brief Function to get an manifest file from multiple Plugininfos.
 * @param pluginInfos array of pluginInfos, to write in the manifest file.
 * @param numPluginInfos Size of the pluginInfos-Array
 * @param ttlFile Stringoutput. This string can be written to a manifest file.
*/
extern "C" std::string getManifestFromMultpleInfos(std::vector<TTLPluginInfo> plugInfos);

/**
 * @brief Writes all plugininfos in the given Array (plugInfos). The Array must have the Size, to match a number of Items, which \see getSizeOfPluginInfos() returns.
 * The returned struct owns not the PluginPtr but the strings. The CStrings must be freed bevore relasing the objects.
 * @param plugInfos Array of Plugininfos with the size of \see getSizeOfPluginInfos()
 * @param binaryPath binary Path reltive in the bundle Folder. This is used to get the reference to the needed binary. 
*/
extern "C" std::vector<TTLPluginInfo> getPluginInfos(std::string binaryPath);

#ifdef __cplusplus
//}
#endif 


#endif //! LV2_TTL_GENERATION_H