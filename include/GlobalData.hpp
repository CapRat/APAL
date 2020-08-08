#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP
#include "XPlug_version.h"
#include <memory>
#include <string>
#include <vector>
namespace XPlug {

class IPlugin;
// class PluginController;
typedef std::shared_ptr<IPlugin> PluginPtr;

class GlobalDataType
{
private:
  std::vector<PluginPtr> registeredPlugins;

public:
  /***************Register Plugin functions****************/

  /**
   * @brief Function, to register a pluign.
   * @param plugin  Plugin to register, this should be a plugin, written by the
   * @return index of the added Plugin. This way more Plugin can be added in one
   * dll.
   */
  size_t registerPlugin(PluginPtr plugin); // re

  /**
   * @brief Returns the number of current registered Plugins. In most cases this
   * should be one
   * @return Number of registered Plugins.
   */
  size_t getNumberOfRegisteredPlugins();

  /**
   * @brief getPlugin Gets the Plugin at the given index. Index should not be
   * greater than @see getNumberOfRegisteredPlugins
   * @param index Index, which Plugin should be chosen.
   * @return A pointer to a Plugin.
   */
  PluginPtr getPlugin(size_t index);

  /**
   * @brief Gets the Plugin with given Name.
   * @param name Name of the Plugin (which is stored in \see IInfoComponent)
   * @return PluginPtr to plugin.
   */
  PluginPtr getPlugin(std::string name);

  // PluginController getPlugin(size_t index);
};

GlobalDataType&
GlobalData();

}

typedef XPlug::GlobalDataType& (*globalDataFncPtr)();

extern "C"
{
  VERSION XPlugGetVersion();
}

#endif //! GLOBAL_DATA_HPP
