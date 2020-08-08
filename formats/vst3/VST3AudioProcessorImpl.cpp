#include "VST3AudioProcessorImpl.hpp"
#include "VST3EditControllerImpl.hpp"
#include <vst/vstspeaker.h>

#include "GlobalData.hpp"
#include "interfaces/IPlugin.hpp"
#include <interfaces/IPlugin.hpp>
#include <interfaces/Ports/IAudioPort.hpp>
#include <tools/PortHandling.hpp>

#include <cmath>
using namespace XPlug;

VST3AudioProccessorImpl::VST3AudioProccessorImpl()
{
  processSetup.maxSamplesPerBlock = 1024;
  processSetup.processMode = Vst::kRealtime;
  processSetup.sampleRate = 44100.0;
  processSetup.symbolicSampleSize = Vst::kSample32;
  this->controllerClass = VST3EditControllerImpl::cid;
}

VST3AudioProccessorImpl::~VST3AudioProccessorImpl() {}

// Geerbt über Component

tresult PLUGIN_API
VST3AudioProccessorImpl::queryInterface(const TUID _iid, void** obj)
{
  // QUERY_INTERFACE(_iid, obj, VST3AudioProccessorImpl::iid,
  // VST3AudioProccessorImpl)
  QUERY_INTERFACE(_iid, obj, IAudioProcessor::iid, IAudioProcessor);
  QUERY_INTERFACE(
    _iid, obj, IProcessContextRequirements::iid, IProcessContextRequirements);
  return Component::queryInterface(_iid, obj);
}

uint32 PLUGIN_API
VST3AudioProccessorImpl::addRef()
{
  return Component::addRef();
}

uint32 PLUGIN_API
VST3AudioProccessorImpl::release()
{
  return Component::release();
}

Speaker
SpeakerPositionToSpeaker(SpeakerPosition pos)
{
  // auto posAsInt =
  // static_cast<std::underlying_type<SpeakerPosition>::type>(pos);
  if (static_cast<std::underlying_type<SpeakerPosition>::type>(pos) < kSpeakerM)
    return static_cast<Speaker>(pos);
  else
    throw NotImplementedException();
}

SpeakerArrangement
SpeakerConfigToSpeakerArrangement(SpeakerConfiguration conf)
{
  if (conf == SpeakerConfiguration::Mono)
    return SpeakerArr::kMono;
  return static_cast<SpeakerArrangement>(conf);
}

SpeakerConfiguration
SpeakerArrangementToSpeakerConfig(SpeakerArrangement arr)
{
  if (arr == SpeakerArr::kMono)
    return SpeakerConfiguration::Mono;
  else
    return static_cast<SpeakerConfiguration>(arr);
}

/** Try to set (from host) a predefined arrangement for inputs and outputs.
The host should always deliver the same number of input and output buses than
the Plug-in needs (see \ref IComponent::getBusCount). The Plug-in returns
kResultFalse if wanted arrangements are not supported. If the Plug-in accepts
these arrangements, it should modify its buses to match the new arrangements
(asked by the host with IComponent::getInfo () or
IAudioProcessor::getBusArrangement ()) and then return kResultTrue. If the
Plug-in does not accept these arrangements, but can adapt its current
arrangements (according to the wanted ones), it should modify its buses
arrangements and return kResultFalse. */

tresult PLUGIN_API
VST3AudioProccessorImpl::setBusArrangements(SpeakerArrangement* inputs,
                                            int32 numIns,
                                            SpeakerArrangement* outputs,
                                            int32 numOuts)
{
  if (numIns < 0 || numOuts < 0)
    return kInvalidArgument;
  auto plug = GlobalData().getPlugin(plugIndex).get();
  if (numIns ==
        (int32)getNumberOfPorts<IAudioPort>(plug, PortDirection::Input) &&
      numOuts ==
        (int32)getNumberOfPorts<IAudioPort>(plug, PortDirection::Output)) {
    for (int32 in = 0; in < numIns; in++) {
      if (SpeakerArrangementToSpeakerConfig(inputs[in]) !=
          getPortAt<IAudioPort>(plug, in, PortDirection::Input)->getConfig())
        return kResultFalse;
    }
    for (int32 out = 0; out < numOuts; out++) {
      if (SpeakerArrangementToSpeakerConfig(outputs[out]) !=
          getPortAt<IAudioPort>(plug, out, PortDirection::Output)->getConfig())
        return kResultFalse;
    }
    return kResultTrue;
  }

  /*
  if (numIns > static_cast<int32> (audioInputs.size ()) ||
  numOuts > static_cast<int32> (audioOutputs.size ()))
  return kResultFalse;

  for (int32 index = 0; index < static_cast<int32> (audioInputs.size ());
  ++index)
  {
  if (index >= numIns)
  break;
  FCast<Vst::AudioBus> (audioInputs[index].get ())->setArrangement
  (inputs[index]);
  }

  for (int32 index = 0; index < static_cast<int32> (audioOutputs.size ());
  ++index)
  {
  if (index >= numOuts)
  break;
  FCast<Vst::AudioBus> (audioOutputs[index].get ())->setArrangement
  (outputs[index]);
  }*/

  return kResultFalse;
}

/** Gets the bus arrangement for a given direction (input/output) and index.
Note: IComponent::getInfo () and IAudioProcessor::getBusArrangement () should be
always return the same information about the buses arrangements. */

tresult PLUGIN_API
VST3AudioProccessorImpl::getBusArrangement(BusDirection dir,
                                           int32 index,
                                           SpeakerArrangement& arr)
{

  /*BusList* busList = getBusList (kAudio, dir);
  if (!busList || busIndex < 0 || static_cast<int32> (busList->size ()) <=
  busIndex) return kInvalidArgument; AudioBus* audioBus = FCast<Vst::AudioBus>
  (busList->at (busIndex)); if (audioBus)
  {
  arr = audioBus->getArrangement ();
  return kResultTrue;
  }
  return kResultFalse;*/

  arr = SpeakerConfigToSpeakerArrangement(
    getPortAt<IAudioPort>(GlobalData().getPlugin(plugIndex).get(),
                          static_cast<size_t>(index),
                          dir == kInput ? PortDirection::Input
                                        : PortDirection::Output)
      ->getConfig());
  return kResultTrue;
}

/** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */

tresult PLUGIN_API
VST3AudioProccessorImpl::canProcessSampleSize(int32 symbolicSampleSize)
{
  if (symbolicSampleSize == SymbolicSampleSizes::kSample32)
    return kResultOk;
  return kResultFalse;
}

/** Gets the current Latency in samples.
The returned value defines the group delay or the latency of the Plug-in. For
example, if the Plug-in internally needs to look in advance (like compressors)
512 samples then this Plug-in should report 512 as latency. If during the use of
the Plug-in this latency change, the Plug-in has to inform the host by using
IComponentHandler::restartComponent (kLatencyChanged), this could lead to audio
playback interruption because the host has to recompute its internal mixer delay
compensation.
Note that for player live recording this latency should be zero or small. */

uint32 PLUGIN_API
VST3AudioProccessorImpl::getLatencySamples()
{
  return 512;
}

/** Called in disable state (setActive not called with true) before
 * setProcessing is called and processing will begin. */

tresult PLUGIN_API
VST3AudioProccessorImpl::setupProcessing(ProcessSetup& newSetup)
{
  processSetup.maxSamplesPerBlock = newSetup.maxSamplesPerBlock;
  processSetup.processMode = newSetup.processMode;
  processSetup.sampleRate = newSetup.sampleRate;

  if (canProcessSampleSize(newSetup.symbolicSampleSize) != kResultTrue)
    return kResultFalse;

  processSetup.symbolicSampleSize = newSetup.symbolicSampleSize;

  return kResultOk;
}

/** Informs the Plug-in about the processing state. This will be called before
any process calls start with true and after with false. Note that setProcessing
(false) may be called after setProcessing (true) without any process calls. In
this call the Plug-in should do only light operation (no memory allocation or
big setup reconfiguration), this could be used to reset some buffers (like Delay
line or Reverb). The host has to be sure that it is called only when the Plug-in
is enable (setActive was called) */

tresult PLUGIN_API VST3AudioProccessorImpl::setProcessing(TBool)
{
  return kResultOk;
}

/** The Process call, where all information (parameter changes, event, audio
 * buffer) are passed. */
#include <vst/ivstevents.h>
tresult PLUGIN_API
VST3AudioProccessorImpl::process(ProcessData& data)
{
  auto plug = GlobalData().getPlugin(plugIndex).get();
  /*if (data.numInputs != getNumberOfPorts<IAudioPort>(plug,
     PortDirection::Input) || data.numOutputs !=
     getNumberOfPorts<IAudioPort>(plug, PortDirection::Output)) return
     kResultFalse;*/
  int32 inputIndex = 0;
  int32 outputIndex = 0;
  iteratePorts<IAudioPort>(
    plug, [&inputIndex, &outputIndex, &data](IAudioPort* p, size_t) {
      p->setSampleSize(data.numSamples);
      if (p->getDirection() == PortDirection::Input
            ? inputIndex < data.numInputs
            : outputIndex < data.numOutputs) {
        AudioBusBuffers& b = p->getDirection() == PortDirection::Input
                               ? data.inputs[inputIndex++]
                               : data.outputs[outputIndex++];
        if (b.numChannels != (int32)p->size())
          return false;
        for (int channelIndex = 0; channelIndex < b.numChannels;
             channelIndex++) {
          // TODO: hier double processing einfügen, wenn implementiert.
          p->at(channelIndex)->feed(b.channelBuffers32[channelIndex]);
        }
      }
      return false;
    });
  if (plug->getFeatureComponent()->supportsFeature(Feature::MidiInput) &&
      data.inputEvents != nullptr) {
    for (int i = 0; i < data.inputEvents->getEventCount(); i++) {
      Event ev;
      MidiMessage msg;

      data.inputEvents->getEvent(i, ev);
      auto cPort = getPortAt<IMidiPort>(
        plug,
        ev.busIndex,
        PortDirection::Input); // TODO: Validate , wather this is absolute
                               // index, or index just out of events.
      //  kNoteOnEvent  kNoteOffEvent  kDataEvent   kPolyPressureEvent
      //  kNoteExpressionValueEvent   kNoteExpressionTextEvent  kChordEvent
      //  kScaleEvent  kLegacyMIDICCOutEven
      switch (ev.type) {
        case ev.kNoteOnEvent:
          msg[0] = static_cast<uint8_t>(MidiEvents::NoteOn) |
                   static_cast<uint8_t>(ev.noteOn.channel);
          msg[1] = ev.noteOn.pitch;
          msg[2] = std::floor(
            ev.noteOn.velocity >= 1.0
              ? 128
              : ev.noteOn.velocity *
                  127.0); // map to 0-127, due to 7 bits midi representation
          break;
        case ev.kNoteOffEvent:
          msg[0] = static_cast<uint8_t>(MidiEvents::NoteOff) |
                   static_cast<uint8_t>(ev.noteOff.channel);
          msg[1] = ev.noteOff.pitch;
          msg[2] = std::floor(
            ev.noteOff.velocity >= 1.0
              ? 128
              : ev.noteOff.velocity *
                  127.0); // map to 0-127, due to 7 bits midi representation
          break;
        default:
          throw NotImplementedException();
          /*case :
              break;*/
      }
      cPort->feed(std::move(msg));
    }
  }

  plug->processAudio();
  if (plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput) &&
      data.outputEvents != nullptr) {
    iteratePorts<IMidiPort>(
      plug, PortDirection::Output, [&data](IMidiPort* p, size_t ind) {
        while (!p->empty()) {
          Event e;
          e.flags = e.kIsLive;
          e.sampleOffset = 0;
          e.busIndex = static_cast<int32_t>(ind);
          e.ppqPosition = 0;
          auto msg = p->get();
          auto midiCmd = static_cast<MidiEvents>(msg[0] & 0xF0);
          uint8_t chan = msg[0] & 0x0F;
          switch (midiCmd) {
            case MidiEvents::NoteOff:
              e.type = e.kNoteOffEvent;
              e.noteOff.channel = chan;
              e.noteOff.pitch = msg[1];
              e.noteOff.velocity = msg[2] / 127.0f;
              e.noteOff.noteId = -1;
              e.noteOff.tuning = 0;
              break;
            case MidiEvents::NoteOn:
              e.type = e.kNoteOnEvent;
              e.noteOn.channel = chan;
              e.noteOn.pitch = msg[1];
              e.noteOn.velocity = msg[2] / 127.0f;
              e.noteOn.noteId = -1;
              e.noteOn.tuning = 0;
              e.noteOn.length = -1;
              break;
            case MidiEvents::ControlChange:
              e.type = e.kLegacyMIDICCOutEvent;
              e.midiCCOut.channel = chan;
              e.midiCCOut.controlNumber = msg[1];
              e.midiCCOut.value = msg[2];
              e.midiCCOut.value2 = msg[2];
              //  e.midiCCOut.value = msg[1];
              //  e.midiCCOut.value2 = msg[2];//no dont use this...
              break;
            default:
              return false;
          }
          data.outputEvents->addEvent(e);
        }
        return false;
      });
  }
  /* while (!p->empty()) {
       auto ev = p->get()
   }
   return false;
   });*/
  // return kNotImplemented;
  return kResultOk;
}

/** Gets tail size in samples. For example, if the Plug-in is a Reverb Plug-in
and it knows that the maximum length of the Reverb is 2sec, then it has to
return in getTailSamples() (in VST2 it was getGetTailSize ()): 2*sampleRate.
This information could be used by host for offline processing, process
optimization and downmix (avoiding signal cut (clicks)). It should return:
- kNoTail when no tail
- x * sampleRate when x Sec tail.
- kInfiniteTail when infinite tail. */

uint32 PLUGIN_API
VST3AudioProccessorImpl::getTailSamples()
{
  return kNoTail;
}

uint32 PLUGIN_API
VST3AudioProccessorImpl::getProcessContextRequirements()
{
  return 0;
}

const ::Steinberg::FUID VST3AudioProccessorImpl::cid(
  INLINE_UID(0x00000100, 0x00011011, 0x00011000, 0x00000110));
