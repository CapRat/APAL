#include <base/PluginBases.hpp>
#include <memory>
using namespace XPlug;

XPlug::LazyPlugin::LazyPlugin()
{
  this->infoComponent = std::make_unique<StaticInfoComponent>(
    "No Name Plugin",
    "urn://LazyPlugin/ToLazyToInit",
    "A simple plugin wich does something... at least it exists",
    "LGPL",
    "Me",
    "http://myhompeage.never.existed.com");

  this->portComponent= std::make_unique<DynamicPortComponent>();
  this->featureComponent = std::make_unique<AutomaticFeatureComponent>(this->portComponent.get());
}

XPlug::LazyPlugin::LazyPlugin(std::vector<std::unique_ptr<IPort>> ports)
  : LazyPlugin()
{
  for (size_t i = 0; i < ports.size(); i++) {
    this->portComponent->addPort(std::move(ports[i]));
  }
  this->featureComponent->detectFeatures(this->getPortComponent());
}

void
XPlug::LazyPlugin::init()
{}

void
XPlug::LazyPlugin::deinit()
{}

void
XPlug::LazyPlugin::activate()
{}

void
XPlug::LazyPlugin::deactivate()
{}
