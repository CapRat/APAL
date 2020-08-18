#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "Types.hpp"

#include "GlobalData.hpp"
#include "IFeatureComponent.hpp"
#include "IInfoComponent.hpp"
#include "Ports/IPortComponent.hpp"
#include <memory>
#include <string>
namespace APAL {
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

/**
 * @brief This is the mainclass for the internal API. IPlugin handles some basic
 * functions like initialisation and deactivation. It also hold references for
 * the required components. The API is designed component based, so the
 * components are quiet important!
 */
class IPlugin
{
public:
  /**
   * @brief Virtual destructor, to make destruction safe.
   */
  virtual ~IPlugin() = default;

  /**
   * @brief Processing audio and MIDI data. This call should do the main
   * processing. Before this function is called, the Plugin is initialized with
   * init and activated.
   */
  virtual void processAudio() = 0;

  /**
   * @brief initialize the plugin. This is happening not on static creation, but
   * on first time when the plugin is loaded.
   */
  virtual void init() = 0;

  /**
   * @brief deinitialize the plugin. Init must be called, before deinit is
   * called.
   */
  virtual void deinit() = 0;

  /**
   * @brief activates the plugin. Is called after init. Can be called after a
   * deactivate call. The default start is a deactivated state.
   */
  virtual void activate() = 0;
  /**
   * @brief deactivates the plugin. Is called after deinit. Can be called after
   * an activate call.
   */
  virtual void deactivate() = 0; // deactivates the plugin (put it to sleep)

  /**
   * @brief Gets a reference to the IInfoComponent.
   * @return a reference to the IInfoComponent. It must not be freed. Its
   * livetime is the same as the IPlugin instance.
   */
  virtual IInfoComponent* getInfoComponent() = 0;

  /**
   * @brief  Gets a reference to the IPortComponent.
   * @return a reference to the IPortComponent. It must not be freed. Its
   * livetime is the same as the IPlugin instance.
   */
  virtual IPortComponent* getPortComponent() = 0;

  /**
   * @brief  Gets a reference to the IFeatureComponent.
   * @return a reference to the IFeatureComponent. It must not be freed. Its
   * livetime is the same as the IPlugin instance.
   */
  virtual IFeatureComponent* getFeatureComponent() = 0;
};

typedef std::shared_ptr<IPlugin> PluginPtr;

}
#endif //! PLUGIN_HPP
