/**
 * @file Exampleplugin, to show the use of an easy Start with LazyPlugin. Easy Midi Messaging Concepts are explained here.
 */
#include <base/PluginBases.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace XPlug;
/**
 * @brief MidiForwarder Example class, which has 1 Midi  Input and 2 Midi
 * Outputs. The Midi in signal is forwarded to the 2 Midi Outputs
 */
class MidiForwarder : public LazyPlugin
{
public:
  /**
   * @brief Standard constructor, which creates on instantiation the needed
   * Ports and Informations.
   */
  MidiForwarder()
  {
    // Adding the needed in and output ports in our portcomponent
    this->portComponent->addPort(
      std::make_shared<QueueMidiPort>("MidiIn", PortDirection::Input));
    this->portComponent->addPort(
      std::make_shared<QueueMidiPort>("MidiOut", PortDirection::Output));
    this->portComponent->addPort(
      std::make_shared<QueueMidiPort>("MidiOut2", PortDirection::Output));

    // Detect the needed Features for this Plugin(exspecially MIDI support)
    this->featureComponent->detectFeatures(this->getPortComponent());

    // Overwrite some default information
    this->infoComponent->pluginName = "MidiForwarder";
    this->infoComponent->creatorName = "Benjamin Heisch";
    this->infoComponent->pluginUri =
      "http://xplug_plug.in/examples/MidiForwarder";
  }
  // Geerbt über IPlugin
  virtual void processAudio() override
  {
    // Get the needed ports, to operate on from the portcomponent.
    auto in0 = getPortAt<IMidiPort>(this, 0, PortDirection::Input);
    auto out0 = getPortAt<IMidiPort>(this, 0, PortDirection::Output);
    auto out1 = getPortAt<IMidiPort>(this, 1, PortDirection::Output);
    // Forward all Messages from the Inputs to the output.
    while (!in0->empty()) {
      auto midiMsg = in0->get();
      auto midiMsg2 = midiMsg;
      out0->feed(std::move(midiMsg));
      out1->feed(std::move(midiMsg2));
    }
  }
  virtual void init() override {}
  virtual void deinit() override {}
  virtual void activate() override {}
  virtual void deactivate() override {}
};
REGISTER_PLUGIN(MidiForwarder);
