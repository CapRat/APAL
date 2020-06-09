#include "GlobalData.hpp"
namespace XPlug {
    GlobalDataType& GlobalData()
    {
        static GlobalDataType* globalDataInstance = new GlobalDataType();
        return *globalDataInstance;
    }


    VERSION XPlugGetVersion()
    {
        static const struct VERSION XPlug_VERSION = { XPlug_VERSION_MAJOR, XPlug_VERSION_MINOR, XPlug_VERSION_PATCH, XPlug_VERSION_TWEAK, XPlug_VERSION_STRING };
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
        return   this->registeredPlugins.at(index);
    }
    /*
    PluginController GlobalDataType::getPlugin(size_t index)
    {
        return  this->registeredPlugins.at(index);
    }
    */
}