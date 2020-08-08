#ifndef EXAMPLE_PLUGIN_HPP
#define EXAMPLE_PLUGIN_HPP
#include <base/LazyPlugin.hpp>
using namespace XPlug;
class VolumePlugin : public LazyPlugin
{
public:
  VolumePlugin();
  // Geerbt über IPlugin
  virtual void processAudio() override;
  virtual void init() override;
  virtual void deinit() override;
  virtual void activate() override;
  virtual void deactivate() override;
  // virtual PluginInfo* getInfoComponent() override;
};

#endif //! EXAMPLE_PLUGIN_HPP