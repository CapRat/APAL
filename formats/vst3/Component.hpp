#ifndef COMPONENT_HPP
#define COMPONENT_HPP
#include <vst/ivstcomponent.h>
using namespace Steinberg;
using namespace Vst;

class Component : public IComponent
{
public:
  Component();
  virtual ~Component() = default;
  // Geerbt über IComponent
  virtual tresult PLUGIN_API queryInterface(const TUID _iid,
                                            void** obj) override;
  virtual uint32 PLUGIN_API addRef() override;
  virtual uint32 PLUGIN_API release() override;
  /** The host passes a number of interfaces as context to initialize the
  Plug-in class.
  @note Extensive memory allocations etc. should be performed in this method
  rather than in the class' constructor! If the method does NOT return
  kResultOk, the object is released immediately. In this case terminate is not
  called! */
  virtual tresult PLUGIN_API initialize(FUnknown* context) override;
  /** This function is called before the Plug-in is unloaded and can be used for
  cleanups. You have to release all references to any host application
  interfaces. */
  virtual tresult PLUGIN_API terminate() override;
  /** Called before initializing the component to get information about the
   * controller class. */
  virtual tresult PLUGIN_API getControllerClassId(TUID classId) override;
  /** Called before 'initialize' to set the component usage (optional). See \ref
   * IoModes */
  virtual tresult PLUGIN_API setIoMode(IoMode mode) override;
  /** Called after the Plug-in is initialized. See \ref MediaTypes,
   * BusDirections */
  virtual int32 PLUGIN_API getBusCount(MediaType type,
                                       BusDirection dir) override;
  /** Called after the Plug-in is initialized. See \ref MediaTypes,
   * BusDirections */
  virtual tresult PLUGIN_API getBusInfo(MediaType type,
                                        BusDirection dir,
                                        int32 index,
                                        BusInfo& bus) override;
  /** Retrieves routing information (to be implemented when more than one
  regular input or output bus exists).
  The inInfo always refers to an input bus while the returned outInfo must refer
  to an output bus! */
  virtual tresult PLUGIN_API getRoutingInfo(RoutingInfo& inInfo,
                                            RoutingInfo& outInfo) override;
  /** Called upon (de-)activating a bus in the host application. The Plug-in
     should only processed an activated bus, the host could provide less see
     \ref AudioBusBuffers in the process call (see \ref
     IAudioProcessor::process) if last buses are not activated */
  virtual tresult PLUGIN_API activateBus(MediaType type,
                                         BusDirection dir,
                                         int32 index,
                                         TBool state) override;
  /** Activates / deactivates the component. */
  virtual tresult PLUGIN_API setActive(TBool state) override;
  /** Sets complete state of component. */
  virtual tresult PLUGIN_API setState(IBStream* state) override;
  /** Retrieves complete state of component. */
  virtual tresult PLUGIN_API getState(IBStream* state) override;

protected:
  int plugIndex = 0; // Index, which internal plugin is used.
  int32 __funknownRefCount;
  FUID controllerClass;
};
#endif //! COMPONENT_HPP
