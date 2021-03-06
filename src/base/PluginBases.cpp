#include <base/PluginBases.hpp>
#include <memory>
#include <random>
using namespace APAL;

APAL::LazyPlugin::LazyPlugin()
{
  std::default_random_engine engine;
  this->infoComponent = std::make_unique<StaticInfoComponent>(
    "No Name Plugin",
    "urn://LazyPlugin/ToLazyToInit",
    "A simple plugin wich does something... at least it exists",
    "LGPL",
    "Me",
    "http://myhompeage.never.existed.com",
    std::uniform_int_distribution<int64_t>()(engine));

  this->portComponent = std::make_unique<DynamicPortComponent>();
  this->featureComponent =
    std::make_unique<AutomaticFeatureComponent>(this->portComponent.get());
}

APAL::LazyPlugin::LazyPlugin(std::vector<std::unique_ptr<IPort>> ports)
  : LazyPlugin()
{
  for (size_t i = 0; i < ports.size(); i++) {
    this->portComponent->addPort(std::move(ports[i]));
  }
  this->featureComponent->detectFeatures(this->getPortComponent());
}

void
APAL::LazyPlugin::init()
{}

void
APAL::LazyPlugin::deinit()
{}

void
APAL::LazyPlugin::activate()
{}

void
APAL::LazyPlugin::deactivate()
{}
