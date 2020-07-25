#include <base/PluginBases.hpp>
#include<memory>
using namespace XPlug;
/*void XPlug::LazyPlugin::processAudio(const std::vector<IPort>& inputs, const std::vector<IPort>& outputs)
{
}*/

XPlug::LazyPlugin::LazyPlugin() : ModularPlugin(
    std::make_unique<StaticInfoComponent>("No Name Plugin", "urn://LazyPlugin/ToLazyToInit", "A simple plugin wich does something... at least it exists", "LGPL", "Me", "http://myhompeage.never.existed.com"),
    std::make_unique<DynamicPortComponent>(),
    std::make_unique<AutomaticFeatureComponent>())
{  }

XPlug::LazyPlugin::LazyPlugin(std::vector<std::unique_ptr<IPort>> ports):LazyPlugin()
{
    for (int i = 0; i < ports.size(); i++) {
        this->portComponent->addPort(std::move(ports[i]));
    }
    this->featureComponent->detectFeatures(this->getPortComponent());
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
