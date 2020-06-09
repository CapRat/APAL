#include "Component.hpp"
#include <string>
#include <pluginterfaces/base/ibstream.h>
#include <Types.hpp>
#include <GlobalData.hpp>
#include <interfaces/IPlugin.hpp>
// TODO: Maybe quick implementation of the Component stuff.
//------------------------------------------------------------------------
Component::Component()
/*    : audioInputs(kAudio, kInput)
    , audioOutputs(kAudio, kOutput)
    , eventInputs(kEvent, kInput)
    , eventOutputs(kEvent, kOutput)*/
{
    __funknownRefCount = 1;
}


tresult PLUGIN_API Component::queryInterface(const TUID _iid, void** obj)
{
    QUERY_INTERFACE(_iid, obj, IComponent::iid, IComponent)
    QUERY_INTERFACE(_iid, obj, IPluginBase::iid, IPluginBase)
    QUERY_INTERFACE(_iid, obj, FUnknown::iid, FUnknown)
    * obj = nullptr;
    return kNoInterface;
}

uint32 PLUGIN_API Component::addRef()
{
    return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

uint32 PLUGIN_API Component::release()
{
    if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0)
    {
        delete this;
        return 0;
    }
    return __funknownRefCount;
}

tresult PLUGIN_API Component::initialize(FUnknown* context)
{
    XPlug::GlobalData().getPlugin(plugIndex)->init();
    return kResultOk;
}

tresult PLUGIN_API Component::terminate()
{
    /*audioInputs.clear();
    audioOutputs.clear();
    eventInputs.clear();
    eventOutputs.clear();*/
    XPlug::GlobalData().getPlugin(plugIndex)->deinit();
    return kResultOk;
}

tresult PLUGIN_API Component::getControllerClassId(TUID classId)
{
    if (controllerClass.isValid())
    {
        controllerClass.toTUID(classId);
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API Component::setIoMode(IoMode mode)
{
    return kNotImplemented;
}

int32 PLUGIN_API Component::getBusCount(MediaType type, BusDirection dir)
{
    if (type == kAudio) {
        if (dir == kInput)
            return XPlug::GlobalData().getPlugin(plugIndex)->getPortComponent()->getNumberOfInputPorts();
        else if (dir)
            return XPlug::GlobalData().getPlugin(plugIndex)->getPortComponent()->getNumberOfOutputPorts();
    }
    else {
        return 0;//other mediatypes not supported yet
    }
}

/**
 BusList* Component::getBusList (MediaType type, BusDirection dir)
{
    if (type == kAudio)
        return dir == kInput ? &audioInputs : &audioOutputs;
    else if (type == kEvent)
        return dir == kInput ? &eventInputs : &eventOutputs;
    return nullptr;
}
*/

tresult PLUGIN_API Component::getBusInfo(MediaType type, BusDirection dir, int32 index, BusInfo& bus)
{
    std::vector<XPlug::Port> ports;
    if (dir == kInput)
        ports = XPlug::GlobalData().getPlugin(plugIndex)->getPortComponent()->getInputPorts();
    else if (dir)
        ports = XPlug::GlobalData().getPlugin(plugIndex)->getPortComponent()->getOutputPorts();

    if (index < 0 || index >=ports.size())
        return kInvalidArgument;
    bus.mediaType = type;
    bus.direction = dir;
    
    ports[index].channels;
    bus.channelCount = ports[index].channels.size();
    

    for (int i = 0; i < sizeof(bus.name)&& i< ports[index].name.size(); i++) {
        bus.name[i] = ports[index].name[i];
    }
    //kAux not supported yet.
    bus.busType = kMain;
    bus.flags =BusInfo::kDefaultActive;
    return kResultOk;
}

tresult PLUGIN_API Component::getRoutingInfo(RoutingInfo& inInfo, RoutingInfo& outInfo)
{
    return kNotImplemented;
}

tresult PLUGIN_API Component::activateBus(MediaType type, BusDirection dir, int32 index, TBool state)
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

    // TODO: Should rly work. But Activation and deactivation is not supported yet..
    return kResultTrue;
}

tresult PLUGIN_API Component::setActive(TBool state)
{
    return kResultTrue;
}

tresult PLUGIN_API Component::setState(IBStream* state)
{
    return kNotImplemented;
}

tresult PLUGIN_API Component::getState(IBStream* state)
{
    return kNotImplemented;
}
