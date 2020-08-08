#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "Types.hpp"

#include "GlobalData.hpp"
#include "IFeatureComponent.hpp"
#include "IInfoComponent.hpp"
#include "Ports/IPortComponent.hpp"
#include <memory>
#include <string>
namespace XPlug {
/**
 * @brief Template Class to Register Plugins in Sourcefiles. The Use of this is
 * to instantiate a static class, which calls the Code in the Connstructor. Call
 * REGISTER_PLUGIN Macro to use this.
 * @tparam PluginType PluginType which should be registered.
 */
template<typename PluginType>
class PluginRegistrator
{
public:
  PluginRegistrator()
  {
    GlobalData().registerPlugin(std::shared_ptr<IPlugin>(new PluginType()));
  }
};

/**
 * Simple Macro, which uses PluginRegistrator. Use this Macro in a Sourcefile.
 */
#define REGISTER_PLUGIN(PluginClassName)                                       \
  static PluginRegistrator<PluginClassName> instance##PluginClassName =        \
    PluginRegistrator<PluginClassName>()
#define EMPTY_STRING ""

// Class which is used to get called from implementation files.
class IPlugin
{
public:
  virtual ~IPlugin() = default;

  /**
   * @brief Procssing Audio and Midi data.
   */
  virtual void processAudio() = 0;

  virtual void
  init() = 0; // initialize the plugin. This is happening not on static
              // creation, but on first time when the plugin is loaded.
  virtual void deinit() = 0; // deinitialize the plugin.

  virtual void activate() = 0;   // activate the plugin(resume from deactivate)
  virtual void deactivate() = 0; // deactivates the plugin (put it to sleep)

  // virtual void registerPlugin() = 0;

  virtual IInfoComponent* getInfoComponent() = 0;
  /*
          virtual size_t getParameterCount() = 0;
          virtual void* getParameter() = 0;
          virtual void setParameter(void*) = 0;*/

  virtual IPortComponent* getPortComponent() = 0;
  virtual IFeatureComponent* getFeatureComponent() = 0;
};

typedef std::shared_ptr<IPlugin> PluginPtr;

}
#endif //! PLUGIN_HPP
