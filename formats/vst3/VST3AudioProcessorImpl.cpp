#include "VST3AudioProcessorImpl.hpp"
#include "VST3EditControllerImpl.hpp"
#include <vst/vstspeaker.h>
#include "interfaces/IPlugin.hpp"
#include "GlobalData.hpp"
#include <interfaces/IPlugin.hpp>
#include <interfaces/IAudioPort.hpp>
using namespace XPlug;


VST3AudioProccessorImpl::VST3AudioProccessorImpl() {
    processSetup.maxSamplesPerBlock = 1024;
    processSetup.processMode = Vst::kRealtime;
    processSetup.sampleRate = 44100.0;
    processSetup.symbolicSampleSize = Vst::kSample32;
    this->controllerClass = VST3EditControllerImpl::cid;
}

VST3AudioProccessorImpl::~VST3AudioProccessorImpl()
{
}

// Geerbt über Component

 tresult PLUGIN_API VST3AudioProccessorImpl::queryInterface(const TUID _iid, void** obj)
{
    //QUERY_INTERFACE(_iid, obj, VST3AudioProccessorImpl::iid, VST3AudioProccessorImpl)
    QUERY_INTERFACE(_iid, obj, IAudioProcessor::iid, IAudioProcessor)
        return Component::queryInterface(_iid, obj);
}

 uint32 PLUGIN_API VST3AudioProccessorImpl::addRef()
{
    return Component::addRef();
}

 uint32 PLUGIN_API VST3AudioProccessorImpl::release()
{
    return Component::release();
}

/** Try to set (from host) a predefined arrangement for inputs and outputs.
The host should always deliver the same number of input and output buses than the Plug-in needs
(see \ref IComponent::getBusCount).
The Plug-in returns kResultFalse if wanted arrangements are not supported.
If the Plug-in accepts these arrangements, it should modify its buses to match the new arrangements
(asked by the host with IComponent::getInfo () or IAudioProcessor::getBusArrangement ()) and then return kResultTrue.
If the Plug-in does not accept these arrangements, but can adapt its current arrangements (according to the wanted ones),
it should modify its buses arrangements and return kResultFalse. */

 tresult PLUGIN_API VST3AudioProccessorImpl::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts)
{
    if (numIns < 0 || numOuts < 0)
        return kInvalidArgument;
    if (numIns == GlobalData().getPlugin(plugIndex)->getPortComponent()->sizeInputPorts() && numOuts == GlobalData().getPlugin(plugIndex)->getPortComponent()->sizeOutputPorts()) {
        for (int in = 0; in < numIns; in++) {
            if (inputs[in] != SpeakerArr::kMono)
                return kResultFalse;
        }
        for (int out = 0; out < numOuts; out++) {
            if (outputs[out] != SpeakerArr::kMono)
                return kResultFalse;
        }
        return kResultTrue;
    }

    /*
    if (numIns > static_cast<int32> (audioInputs.size ()) ||
    numOuts > static_cast<int32> (audioOutputs.size ()))
    return kResultFalse;

    for (int32 index = 0; index < static_cast<int32> (audioInputs.size ()); ++index)
    {
    if (index >= numIns)
    break;
    FCast<Vst::AudioBus> (audioInputs[index].get ())->setArrangement (inputs[index]);
    }

    for (int32 index = 0; index < static_cast<int32> (audioOutputs.size ()); ++index)
    {
    if (index >= numOuts)
    break;
    FCast<Vst::AudioBus> (audioOutputs[index].get ())->setArrangement (outputs[index]);
    }*/

    return kResultFalse;

}

/** Gets the bus arrangement for a given direction (input/output) and index.
Note: IComponent::getInfo () and IAudioProcessor::getBusArrangement () should be always return the same
information about the buses arrangements. */

 tresult PLUGIN_API VST3AudioProccessorImpl::getBusArrangement(BusDirection dir, int32 index, SpeakerArrangement& arr)
{

    /*BusList* busList = getBusList (kAudio, dir);
    if (!busList || busIndex < 0 || static_cast<int32> (busList->size ()) <= busIndex)
    return kInvalidArgument;
    AudioBus* audioBus = FCast<Vst::AudioBus> (busList->at (busIndex));
    if (audioBus)
    {
    arr = audioBus->getArrangement ();
    return kResultTrue;
    }
    return kResultFalse;*/

    arr = SpeakerArr::kMono;
    return kResultTrue;
}

/** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */

 tresult PLUGIN_API VST3AudioProccessorImpl::canProcessSampleSize(int32 symbolicSampleSize)
{
    /*if (symbolicSampleSize == 512)
        return kResultOk;
    return kResultFalse;*/
     return kResultOk;
}

/** Gets the current Latency in samples.
The returned value defines the group delay or the latency of the Plug-in. For example, if the Plug-in internally needs
to look in advance (like compressors) 512 samples then this Plug-in should report 512 as latency.
If during the use of the Plug-in this latency change, the Plug-in has to inform the host by
using IComponentHandler::restartComponent (kLatencyChanged), this could lead to audio playback interruption
because the host has to recompute its internal mixer delay compensation.
Note that for player live recording this latency should be zero or small. */

 uint32 PLUGIN_API VST3AudioProccessorImpl::getLatencySamples()
{
    return 512;
}

/** Called in disable state (setActive not called with true) before setProcessing is called and processing will begin. */

 tresult PLUGIN_API VST3AudioProccessorImpl::setupProcessing(ProcessSetup& newSetup)
{
    processSetup.maxSamplesPerBlock = newSetup.maxSamplesPerBlock;
    processSetup.processMode = newSetup.processMode;
    processSetup.sampleRate = newSetup.sampleRate;

    if (canProcessSampleSize(newSetup.symbolicSampleSize) != kResultTrue)
        return kResultFalse;

    processSetup.symbolicSampleSize = newSetup.symbolicSampleSize;

    return kResultOk;
}

/** Informs the Plug-in about the processing state. This will be called before any process calls
start with true and after with false.
Note that setProcessing (false) may be called after setProcessing (true) without any process
calls. In this call the Plug-in should do only light operation (no memory allocation or big
setup reconfiguration), this could be used to reset some buffers (like Delay line or Reverb).
The host has to be sure that it is called only when the Plug-in is enable (setActive was
called) */

 tresult PLUGIN_API VST3AudioProccessorImpl::setProcessing(TBool state)
{
    return kNotImplemented;
}

/** The Process call, where all information (parameter changes, event, audio buffer) are passed. */
#include <vst/ivstevents.h>
 tresult PLUGIN_API VST3AudioProccessorImpl::process(ProcessData& data)
{
     auto portComp= GlobalData().getPlugin(plugIndex)->getPortComponent();
     if (data.numInputs != portComp->sizeInputPorts() || data.numOutputs != portComp->sizeOutputPorts())
         return kResultFalse;

     for (int i = 0; i < data.numInputs; i++) {
         auto p = dynamic_cast<IAudioPort*>(portComp->inputPortAt(i));
         if (data.inputs[i].numChannels !=p->size())
             return kResultFalse;
         p->setSampleSize(data.numSamples);
         for (int channelIndex = 0; channelIndex < data.inputs[i].numChannels; channelIndex++) {
             static IAudioChannel::AudioChannelData channelData;
             channelData = { data.inputs[i].channelBuffers32[channelIndex] , data.inputs[i].channelBuffers64[channelIndex] };
             p->typesafeAt(channelIndex)->feed(&channelData);
         }
     }
     for (int i = 0; i < data.numOutputs; i++) {
         auto p = dynamic_cast<IAudioPort * >(portComp->outputPortAt(i));
        
         if (data.outputs[i].numChannels != p->size())
             return kResultFalse;
         p->setSampleSize(data.numSamples);
         for (int channelIndex = 0; channelIndex < data.outputs[0].numChannels; channelIndex++) {
             static IAudioChannel::AudioChannelData channelData2;
             channelData2 = { data.inputs[i].channelBuffers32[channelIndex] , data.inputs[i].channelBuffers64[channelIndex] };
             p->typesafeAt(channelIndex)->feed(&channelData2);
         }
     }
      
     GlobalData().getPlugin(0)->processAudio();
    //return kNotImplemented;
    return kResultOk;
}

/** Gets tail size in samples. For example, if the Plug-in is a Reverb Plug-in and it knows that
the maximum length of the Reverb is 2sec, then it has to return in getTailSamples()
(in VST2 it was getGetTailSize ()): 2*sampleRate.
This information could be used by host for offline processing, process optimization and
downmix (avoiding signal cut (clicks)).
It should return:
- kNoTail when no tail
- x * sampleRate when x Sec tail.
- kInfiniteTail when infinite tail. */

 uint32 PLUGIN_API VST3AudioProccessorImpl::getTailSamples()
{
    return kNoTail;
}

const ::Steinberg::FUID VST3AudioProccessorImpl::cid(INLINE_UID(0x00000100, 0x00011011, 0x00011000, 0x00000110));
