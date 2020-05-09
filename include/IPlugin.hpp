#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "Types.hpp"

#include <string>
#include <memory>
#include "GlobalData.hpp"
template<typename PluginType>
class PluginRegistrator {
public:
    PluginRegistrator() {
        GlobalData().registerPlugin(std::shared_ptr<IPlugin>(new PluginType()));
    }
};

#define REGISTER_PLUGIN(PluginClassName) static PluginRegistrator<##PluginClassName##> instance=PluginRegistrator<##PluginClassName##>()
#define EMPTY_STRING ""
struct PluginInfo {
    std::string name= EMPTY_STRING;
    std::string copyright = EMPTY_STRING;
    std::string creater = EMPTY_STRING;
    bool hasUI=false;
    int numAudioInputPorts;
    int numAudioOutputPorts;
    int numMidiInputPorts;
    int numMidiOutputPorts;
};


class Port {
public:
    bool input;
    bool output;
    bool midi;
    std::string name;
    float* data;
};


class IPlugin {
public:

    virtual ~IPlugin() = default;

    virtual void processAudio(std::vector<audio_data> inputs, std::vector<audio_data> outputs) = 0;
    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void registerPlugin() = 0;

    virtual PluginInfo getPluginInfo() = 0;
    virtual void setPluginInfo(PluginInfo inf) = 0;
    virtual void updatePluginInfo() = 0;

    virtual bool hasUI()=0;

    virtual size_t getParameterCount() = 0;
    virtual void* getParameter()=0;
    virtual void setParameter()=0;

};

typedef std::shared_ptr<IPlugin> PluginPtr;




#endif //! PLUGIN_HPP
