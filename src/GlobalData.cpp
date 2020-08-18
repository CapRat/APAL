#include <GlobalData.hpp>
#include <interfaces/IPlugin.hpp>
namespace APAL {
GlobalDataType&
GlobalData()
{
  static GlobalDataType* globalDataInstance = new GlobalDataType();
  return *globalDataInstance;
}

VERSION
APALGetVersion()
{
  // struct my_struct s2 = { "string literal" };
  static const struct VERSION APAL_VERSION = { APAL_VERSION_MAJOR,
                                                APAL_VERSION_MINOR,
                                                APAL_VERSION_PATCH,
                                                APAL_VERSION_TWEAK,
                                                "test" };
  return APAL_VERSION;
}

size_t
GlobalDataType::registerPlugin(PluginPtr plugin)
{
  size_t index = this->getNumberOfRegisteredPlugins();
  this->registeredPlugins.push_back(plugin);
  return index;
}

size_t
GlobalDataType::getNumberOfRegisteredPlugins()
{
  return this->registeredPlugins.size();
}

PluginPtr
GlobalDataType::getPlugin(size_t index)
{
  return this->registeredPlugins.at(index);
}

PluginPtr
GlobalDataType::getPlugin(std::string name)
{
  for (auto plug : this->registeredPlugins) {
    if (plug->getInfoComponent()->getPluginName() == name)
      return plug;
  }
  return nullptr;
}
}

VERSION
APALGetVersion()
{
  return APAL_VERSION;
}
APAL::GlobalDataType&
APALGlobalData()
{
  return APAL::GlobalData();
}