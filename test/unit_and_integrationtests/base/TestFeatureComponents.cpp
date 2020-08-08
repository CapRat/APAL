#include "CatchTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/FeatureComponents.hpp>
#include <base/PortComponents.hpp>
#include <base/Ports.hpp>
using namespace XPlug;
TEST_CASE("Test IFeatureInterface Methods")
{
  DynamicFeatureComponent comp{ { Feature::MidiInput } };
  StaticPortComponent<4> staticPortCompWithMidiIO{
    std::make_shared<MonoPort>("testIn0", PortDirection::Input),
    std::make_shared<MonoPort>("testOut0", PortDirection::Output),
    std::make_shared<QueueMidiPort>("midiIn0", PortDirection::Input),
    std::make_shared<QueueMidiPort>("midiOut0", PortDirection::Output)
  };
  StaticPortComponent<3> staticPortCompWithMidiI{
    std::make_shared<MonoPort>("testIn0", PortDirection::Input),
    std::make_shared<MonoPort>("testOut0", PortDirection::Output),
    std::make_shared<QueueMidiPort>("midiIn0", PortDirection::Input),
  };
  AutomaticFeatureComponent autoFeatCompMidiIO(&staticPortCompWithMidiIO);
  AutomaticFeatureComponent autoFeatCompMidiI(&staticPortCompWithMidiI);
  testIFeatureComponent(&comp);
  testIFeatureComponent(&autoFeatCompMidiIO);
  testIFeatureComponent(&autoFeatCompMidiI);
  INFO("Autofeature component, doesnt work!");
  REQUIRE(autoFeatCompMidiIO.supportsFeature(Feature::MidiInput));
  REQUIRE(autoFeatCompMidiIO.supportsFeature(Feature::MidiOutput));
  REQUIRE(autoFeatCompMidiI.supportsFeature(Feature::MidiInput));
  REQUIRE(!autoFeatCompMidiI.supportsFeature(Feature::MidiOutput));
}