#ifndef GLOBAL_DATA_HPP
#define GLOBAL_DATA_HPP
#include "XPlug_version.h"
#include <memory>
#include <vector>



class IPlugin;
//class PluginController;
typedef std::shared_ptr<IPlugin> PluginPtr;

class GlobalDataType{
private :
	
	std::vector<PluginPtr> registeredPlugins;
public:

	/***************Register Plugin functions****************/
	/**
	 * @ brief Function, to register a pluign.
	 *
	 * @ param plugin Plugin to register, this should be a plugin, written by the user.
	 *
	 * @ return index of the added Plugin. This way more Plugin can be added in one dll. 
	 */
	size_t registerPlugin(PluginPtr plugin); // re
	
	/**
	 * @ brief Returns the number of current registered Plugins. In most cases this should be one
	 *
	 * @ return Number of registered Plugins.
	 */
	size_t getNumberOfRegisteredPlugins();
	
	/**
	 * @ brief 
	 *
	 * @ param index
	 *
	 * @ return 
	 */
	PluginPtr getPlugin(size_t index);

	//PluginController getPlugin(size_t index);
};


GlobalDataType& GlobalData();

extern "C" {
VERSION XPlugGetVersion();
}

#endif //! GLOBAL_DATA_HPP
