#ifndef IFEATURE_COMPONENT_HPP
#define IFEATURE_COMPONENT_HPP
#include <string>
namespace APAL {
/**
 * @brief Features which
 */
enum class Feature
{
  MultiPort,  // Indicates, weather a Plugin supports of having more than one in
              // and output.
  GUISupport, // Indicates, weather a Plugin supports having a graphical
              // userinterface.
  HardRTCapable,
  MidiInput, // INdicates, that MidiInput is available in this plugin.
  MidiOutput // INdicates, that MidiOutput is available in this plugin.
};

/**
 * @brief InterfaceComponent, wich identifies which Features the Plugin
 * supports.
 */
class IFeatureComponent
{
public:
  /**
   * @brief Function, which reports, weather a Feature is supported or not. So
   * the formatlibrary calls this function to tests if the plugin make use of a
   * functionality or not. This gives the formatlibrary the ability to opimize
   * processtime.
   * @param feature feature which should be supported or not.
   * @return if the feature is supported true is returned and false if
   * otherwhise.
   */
  virtual bool supportsFeature(Feature feature) = 0;

  /**
   * @brief  Method, which is  used speacially in DEBUG builds. This Method is
   * called, when an Pluginformat should use a given Feature, which leads to
   * strange workarrounds effects. Like when VST2 should use multiple ports,
   * which are not mono. VST2 just supports 1 In and 1 Out, so just these Ports
   * are used.
   * @param feature Feature, which is not supported.
   * @param format format, which cant support the given Feature.
   */
  virtual void formatNotSupportedFeature(Feature feature,
                                         std::string format) = 0;
};

}

#endif //! IFEATURE_COMPONENT_HPP