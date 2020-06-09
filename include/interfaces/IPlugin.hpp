#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "Types.hpp"

#include "GlobalData.hpp"
#include "interfaces/IPortComponent.hpp"
#include <memory>
#include <string>
namespace XPlug {
/**
     * @brief Template Class to Register Plugins in Sourcefiles. The Use of this is to instantiate a static class, which calls the Code in the Connstructor. Call REGISTER_PLUGIN Macro to use this.
     * @tparam PluginType PluginType which should be registered.
     */
template <typename PluginType>
class PluginRegistrator {
public:
    PluginRegistrator()
    {
        GlobalData().registerPlugin(std::shared_ptr<IPlugin>(new PluginType()));
    }
};

/**
     * Simple Macro, which uses PluginRegistrator. Use this Macro in a Sourcefile.
     **/
#define REGISTER_PLUGIN(PluginClassName) static PluginRegistrator<PluginClassName> instance = PluginRegistrator<PluginClassName>()
#define EMPTY_STRING ""

struct PluginInfo {
    PluginInfo(std::string name = "", std::string description = "", std::string copyright = "", std::string creater = "", std::string url = "", bool hasUI = false)
    {
        this->name = name;
        this->description = description;
        this->copyright = copyright;
        this->creater = creater;
        this->url = url;
        this->hasUI = hasUI;
    }
    /**
         * @brief Name of the Plugin
         */
    std::string name = EMPTY_STRING;

    /**
         * @brief Description or Label of the Plugin
         */
    std::string description = EMPTY_STRING;

    /**
         * @brief Copyright of the Plugin.
         */
    std::string copyright = EMPTY_STRING;

    /**
         * @brief Creator or Vendor from the Plugin
         */
    std::string creater = EMPTY_STRING;
    /**
         * @brief URL of the current Plugin. In LV2 its used for identification. In other formats it should be just an url to get help and information from.
         */
    std::string url = EMPTY_STRING;

    /**
         * @brief Value, weather the current Plugin supports an GUI. CUrrently not supported.
         */
    bool hasUI = false;
};

// Class which is used to get called from implementation files.
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual void processAudio(const std::vector<Port>& inputs, std::vector<Port>& outputs) = 0;

    virtual void init() = 0; //initialize the plugin. This is happening not on static creation, but on first time when the plugin is loaded.
    virtual void deinit() = 0; //deinitialize the plugin.

    virtual void activate() = 0; // activate the plugin(resume from deactivate)
    virtual void deactivate() = 0; // deactivates the plugin (put it to sleep)
    //virtual void registerPlugin() = 0;

    virtual PluginInfo* getPluginInfo() = 0;

    virtual size_t getParameterCount() = 0;
    virtual void* getParameter() = 0;
    virtual void setParameter(void*) = 0;

    virtual IPortComponent* getPortComponent() = 0;
};

typedef std::shared_ptr<IPlugin> PluginPtr;

}
#endif //! PLUGIN_HPP
