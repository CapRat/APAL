#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP
#include "generated/XPlug_version.h"
#include <memory>
#include <vector>
class IPlugin;
typedef std::shared_ptr<IPlugin> PluginPtr;

class GlobalDataType{
private :
	
	std::vector<PluginPtr> registeredPlugins;
public:

	/***************Register Plugin functions****************/
	size_t registerPlugin(PluginPtr plugin);
	size_t getNumberOfRegisteredPlugins();
	PluginPtr getPlugin(size_t index);
};

GlobalDataType& GlobalData();

extern "C" {
VERSION XPlugGetVersion();
}

#endif //! GLOBAL_DATA_HPP
