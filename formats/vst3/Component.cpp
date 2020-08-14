#include "Component.hpp"
#include <GlobalData.hpp>
#include <Types.hpp>
#include <interfaces/IPlugin.hpp>
#include <pluginterfaces/base/ibstream.h>
#include <string>
#include <tools/PortHandling.hpp>
// TODO: Maybe quick implementation of the Component stuff.
//------------------------------------------------------------------------
Component::Component()
  : __funknownRefCount(1)
{}

tresult PLUGIN_API
Component::queryInterface(const TUID _iid, void** obj)
{
  QUERY_INTERFACE(_iid, obj, IComponent::iid, IComponent)
  QUERY_INTERFACE(_iid, obj, IPluginBase::iid, IPluginBase)
  QUERY_INTERFACE(_iid, obj, FUnknown::iid, FUnknown)
  *obj = nullptr;
  return kNoInterface;
}

uint32 PLUGIN_API
Component::addRef()
{
  return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

uint32 PLUGIN_API
Component::release()
{
  if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0) {
    delete this;
    return 0;
  }
  return __funknownRefCount;
}

tresult PLUGIN_API
Component::initialize(FUnknown*)
{
  XPlug::GlobalData().getPlugin(plugIndex)->init();
  return kResultOk;
}

tresult PLUGIN_API
Component::terminate()
{
  XPlug::GlobalData().getPlugin(plugIndex)->deinit();
  return kResultOk;
}

tresult PLUGIN_API
Component::getControllerClassId(TUID classId)
{
  if (controllerClass.isValid()) {
    controllerClass.toTUID(classId);
    return kResultTrue;
  }
  return kResultFalse;
}

tresult PLUGIN_API Component::setIoMode(IoMode)
{
  return kNotImplemented;
}

int32 PLUGIN_API
Component::getBusCount(MediaType type, BusDirection dir)
{
  if (type == kAudio) {
    return static_cast<int32>(XPlug::getNumberOfPorts<XPlug::IAudioPort>(
      XPlug::GlobalData().getPlugin(plugIndex).get(),
      dir == kInput ? XPlug::PortDirection::Input
                    : XPlug::PortDirection::Output));
  } else if (type == kEvent) {
    return static_cast<int32>(XPlug::getNumberOfPorts<XPlug::IMidiPort>(
      XPlug::GlobalData().getPlugin(plugIndex).get(),
      dir == kInput ? XPlug::PortDirection::Input
                    : XPlug::PortDirection::Output));
  } else {
    return 0;
  }
}


tresult PLUGIN_API
Component::getBusInfo(MediaType type,
                      BusDirection dir,
                      int32 index,
                      BusInfo& bus)
{
  XPlug::IPort* p = nullptr;
  if (index < 0 || index > getBusCount(type, dir))
    return kInvalidArgument;
  if (type == kAudio) {
    auto ap = XPlug::getPortAt<XPlug::IAudioPort>(
      XPlug::GlobalData().getPlugin(plugIndex).get(),
      static_cast<size_t>(index),
      dir == kInput ? XPlug::PortDirection::Input
                    : XPlug::PortDirection::Output);
    p = ap;
    bus.channelCount = static_cast<int32>(ap->size());
  } else if (type == kEvent) {
    auto mp = XPlug::getPortAt<XPlug::IMidiPort>(
      XPlug::GlobalData().getPlugin(plugIndex).get(),
      static_cast<size_t>(index),
      dir == kInput ? XPlug::PortDirection::Input
                    : XPlug::PortDirection::Output);
    p = mp;
    bus.channelCount = 16;
  } else {
    return kNotImplemented;
  }

  bus.mediaType = type;
  bus.direction = dir;

  for (size_t i = 0; i < sizeof(bus.name) && i < p->getPortName().size(); i++) {
    bus.name[i] = p->getPortName()[i];
  }
  // kAux not supported yet.
  bus.busType = kMain;
  bus.flags = BusInfo::kDefaultActive;
  return kResultOk;
}


tresult PLUGIN_API
Component::getRoutingInfo(RoutingInfo&, RoutingInfo&)
{
  return kNotImplemented;
}

// tresult PLUGIN_API Component::activateBus(MediaType type, BusDirection dir,
// int32 index, TBool state)
tresult PLUGIN_API Component::activateBus(MediaType, BusDirection, int32, TBool)
{
  /*
  if (index < 0)
      return kInvalidArgument;
  BusList* busList = getBusList (type, dir);
  if (busList == nullptr)
      return kInvalidArgument;
  if (index >= static_cast<int32> (busList->size ()))
      return kInvalidArgument;

  Bus* bus = busList->at (index);
  bus->setActive (state);
  return kResultTrue;*/

  // TODO: Should rly work. But Activation and deactivation is not supported
  // yet..
  return kResultTrue;
}

tresult PLUGIN_API Component::setActive(TBool)
{
  return kResultTrue;
}

tresult PLUGIN_API
Component::setState(IBStream*)
{
  return kNotImplemented;
}

tresult PLUGIN_API
Component::getState(IBStream*)
{
  return kNotImplemented;
}
