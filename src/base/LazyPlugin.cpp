#include <base/LazyPlugin.hpp>
using namespace XPlug;
/*void XPlug::LazyPlugin::processAudio(const std::vector<IPort>& inputs, const std::vector<IPort>& outputs)
{
}*/

XPlug::LazyPlugin::LazyPlugin()
{
    this->inf = PluginInfo{ "No Name Plugin" ,"a random dude from the internet","a random dude from the internet","","urn://LazyPlugin/ToLazyToInit", false };
}

XPlug::LazyPlugin::LazyPlugin(std::vector<std::unique_ptr<IPort>> ports)
{
    this->inf = PluginInfo{ "No Name Plugin" ,"a random dude from the internet","a random dude from the internet","","urn://LazyPlugin/ToLazyToInit", false };
    for (int i = 0; i < ports.size(); i++) {
        this->portComponent.addPort(std::move(ports[i]));
    }
    this->featureComp.detectFeatures(this->getPortComponent());
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
   // static PluginInfo* inf = new PluginInfo{ "No Name Plugin" ,"a random dude from the internet","a random dude from the internet","","urn://LazyPlugin/ToLazyToInit", false };
    return &inf;
}


IPortComponent* XPlug::LazyPlugin::getPortComponent()
{
    return &portComponent;
}
IFeatureComponent* XPlug::LazyPlugin::getFeatureComponent() {
    return &featureComp;
}