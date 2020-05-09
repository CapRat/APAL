#include "GlobalData.hpp"

GlobalDataType& GlobalData()
{ 
   static GlobalDataType* globalDataInstance = new GlobalDataType();
   return *globalDataInstance;
}


VERSION XPlugGetVersion()
{
    return XPlug_VERSION;
}

size_t GlobalDataType::registerPlugin(PluginPtr plugin)
{
    size_t index = this->getNumberOfRegisteredPlugins();
    this->registeredPlugins.push_back(plugin);
    return index;
}


size_t GlobalDataType::getNumberOfRegisteredPlugins()
{
    return  this->registeredPlugins.size();
}

PluginPtr GlobalDataType::getPlugin(size_t index)
{
    return  this->registeredPlugins.at(index);
}
