#include <base/LazyPlugin.hpp>
using namespace XPlug;
/*void XPlug::LazyPlugin::processAudio(const std::vector<Port>& inputs, const std::vector<Port>& outputs)
{
}*/

XPlug::LazyPlugin::LazyPlugin()
{
    
}

XPlug::LazyPlugin::LazyPlugin(std::vector<Port> ports)
{
    for (Port& p : ports) {
        this->portComponent.addPort(p);
    }
}

void XPlug::LazyPlugin::init()
{
}

void XPlug::LazyPlugin::deinit()
{
}

void XPlug::LazyPlugin::activate()
{
}

void XPlug::LazyPlugin::deactivate()
{
}

PluginInfo* XPlug::LazyPlugin::getPluginInfo()
{
    static PluginInfo* inf = new PluginInfo{ "No Name Plugin" ,"a random dude from the internet","a random dude from the internet","","urn://LazyPlugin/ToLazyToInit", false };
    return inf;
}

size_t XPlug::LazyPlugin::getParameterCount()
{
    return size_t();
}

void* XPlug::LazyPlugin::getParameter()
{
    return nullptr;
}

void XPlug::LazyPlugin::setParameter(void*)
{
}


IPortComponent* XPlug::LazyPlugin::getPortComponent()
{
    return &portComponent;
}
