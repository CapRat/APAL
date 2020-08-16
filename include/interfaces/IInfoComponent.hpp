#ifndef I_INFO_COMPONENT_HPP
#define I_INFO_COMPONENT_HPP
#include <Types.hpp>
namespace XPlug {
/**
 * @brief Componentinterface, which is used to retreive Information over the
 * Plugin. Maybe in the Future it can be extended with generic information, but
 * a better concept is needed. Speacially because it can be used to transfer
 * IDs, which are needed py formatlibraries.
 */
class IInfoComponent
{
public:
  /**
   * @brief Gets the Name of the Plugin.
   * @return Name of the given Plugin.
   */
  virtual std::string_view getPluginName() = 0;

  /**
   * @brief Gets the Plugin URI. This can be an Website, but doesnt have to.
   * In LV2 this URI is used to identify the Plugin.
   * @return URI of the Plugin.
   */
  virtual std::string_view getPluginURI() = 0;

  /**
   * @brief Gets the Description of the Plugin.
   * @return Description of the Plugin.
   */
  virtual std::string_view getPluginDescription() = 0;

  /**
   * @brief Gets the Copyright or Licence of the Plugin.
   * @return Copyright or Licence of the Plugin.
   */
  virtual std::string_view getPluginCopyright() = 0;
  /**
   * @brief Gets the ID of the Plugin, which is used in the format libraries.
   * @return returns the id of the plugin.
  */
  virtual int64_t getID() = 0;
  /********************Creator/Vendor information*********************/
  /**
   * @brief Gets the Name of the creator, vendor or developer.
   * @return Name of the creator, vendor or developer.
   */
  virtual std::string_view getCreatorName() = 0;

  /**
   * @brief Gets the URL from the plugincreator or pluginvendor
   * @return URL from the plugincreator or pluginvendor
   */
  virtual std::string_view getCreatorURL() = 0;
};

}
#endif //! I_INFO_COMPONENT_HPP