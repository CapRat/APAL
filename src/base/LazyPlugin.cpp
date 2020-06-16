#include <base/LazyPlugin.hpp>
using namespace XPlug;
/*void XPlug::LazyPlugin::processAudio(const std::vector<IPort>& inputs, const std::vector<IPort>& outputs)
{
}*/

XPlug::LazyPlugin::LazyPlugin()
{
    
}

XPlug::LazyPlugin::LazyPlugin(std::vector<std::unique_ptr<IPort>> ports)
{
    for (int i = 0; i < ports.size(); i++) {
        this->portComponent.addPort(std::move(ports[i]));
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
