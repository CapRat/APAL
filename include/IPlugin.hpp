#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "Types.hpp"

#include "GlobalData.hpp"
#include <memory>
#include <string>
template <typename PluginType>
class PluginRegistrator {
public:
    PluginRegistrator()
    {
        GlobalData().registerPlugin(std::shared_ptr<IPlugin>(new PluginType()));
    }
};

#define REGISTER_PLUGIN(PluginClassName) static PluginRegistrator<PluginClassName> instance = PluginRegistrator<PluginClassName>()
#define EMPTY_STRING ""

struct PluginInfo {
    std::string name = EMPTY_STRING;
    std::string copyright = EMPTY_STRING;
    std::string creater = EMPTY_STRING;
    bool hasUI = false;
    size_t numberOfParameters;
};

class Parameter {
};

class Port {
public:
    bool input;
    bool output;
    bool midi;
    std::string name;
    float* data;
};

// Class which is used to get called from implementation files.
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual void processAudio(std::vector<audio_data> inputs, std::vector<audio_data> outputs) = 0;
    virtual void init() = 0; //initialize the plugin. This is happening not on static creation, but on first time when the plugin is loaded.
    virtual void deinit() = 0; //deinitialize the plugin.
    virtual void activate() = 0; // activate the plugin(resume from deactivate)
    virtual void deactivate() = 0; // deactivates the plugin (put it to sleep)
    //virtual void registerPlugin() = 0;

    virtual PluginInfo getPluginInfo() = 0;

    virtual size_t getParameterCount() = 0;
    virtual void* getParameter() = 0;
    virtual void setParameter(void*) = 0;

    virtual std::vector<Port> getPorts() = 0;
};

// User Plugin Interface. (leicht zu nutzen, für den Nutzer)

// Control Plugin Interface (internes interface, welches von den formaten angesprochen wird)

typedef std::shared_ptr<IPlugin> PluginPtr;

#endif //! PLUGIN_HPP
