#include "InterfaceTestMethods.hpp"
#include "CatchTools.hpp"
#include "interfaces/IFeatureComponent.hpp"
#include "interfaces/Ports/IAudioPort.hpp"
#include "interfaces/Ports/IMidiPort.hpp"
using namespace XPlug;
#define LAST_FEATURE Feature::MidiOutput

void
testIFeatureComponent(XPlug::IFeatureComponent* featComp)
{
  // Just test, that no exception is thrown
  for (size_t i = 0; i <= static_cast<size_t>(LAST_FEATURE) + 1; i++) {
    featComp->supportsFeature(static_cast<Feature>(i));
    try {
      featComp->formatNotSupportedFeature(static_cast<Feature>(i), "test");
    } catch (std::exception& e) {
    }
  }
  REQUIRE_MESSAGE(featComp->supportsFeature(static_cast<Feature>(
                    static_cast<size_t>(LAST_FEATURE) + 1)) == false,
                  "Non defined Feature should be not supported. Otherwhise you "
                  "would be rly ... good?");
}

void
testIInfoComponent(XPlug::IInfoComponent* infoComp)
{
  REQUIRE_MESSAGE(!infoComp->getCreatorName().empty(), "creator name is empty");
  REQUIRE_MESSAGE(!infoComp->getCreatorURL().empty(), "creator url is empty");
  REQUIRE_MESSAGE(!infoComp->getPluginCopyright().empty(),
                  "plugin copyright is empty");
  REQUIRE_MESSAGE(!infoComp->getPluginDescription().empty(),
                  "plugin description is empty");
  REQUIRE_MESSAGE(!infoComp->getPluginName().empty(), "plugin Name is empty");
  REQUIRE_MESSAGE(!infoComp->getPluginURI().empty(), "plugin uri is empty");
}

void
testIPort(XPlug::IPort* port)
{
  REQUIRE_MESSAGE(port != nullptr,
                  "Error, dont get nullptr, if size says there should be sth");
  REQUIRE_MESSAGE((port->getDirection() == PortDirection::Input ||
                   port->getDirection() == PortDirection::Output),
                  "Ports can either be direction input or output. Nothing else "
                  "at the moment.");
  REQUIRE_MESSAGE(!port->getPortName().empty(),
                  "Give your Port a name.. .At least an non null str");
  switch (port->getType()) {
    case PortType::Audio:
      testIAudioPort(dynamic_cast<IAudioPort*>(port));
      break;
    case PortType::MIDI:
      testIMidiPort(dynamic_cast<IMidiPort*>(port));
      break;
    case PortType::None:
      REQUIRE_MESSAGE(false, "There should be no use of None, right now");
      break;
    default:
      REQUIRE_MESSAGE(false, "No implemented version of PortType is used.");
  }
}
void
testIAudioPort(XPlug::IAudioPort* aPort)
{
  REQUIRE_MESSAGE(aPort != nullptr, "Null Audioport is definitly wrong");
}
void
testIMidiPort(XPlug::IMidiPort* mPort)
{
  REQUIRE_MESSAGE(mPort != nullptr, "Null Midiport is definitly wrong");
  // MidiMessage x = { 0x1,0x2,0x3 };
}

void
testIPortComponent(XPlug::IPortComponent* portComp)
{
  //  INFO("Error, there should nothing above an index size.");
  //  REQUIRE_THROWS(portComp->at(portComp->size()) == nullptr);
  for (size_t i = 0; i < portComp->size(); i++) {
    testIPort(portComp->at(i));
  }
}

void
testIPlugin(XPlug::IPlugin* plug)
{
  REQUIRE_NOTHROW(plug->init());
  REQUIRE_NOTHROW(plug->activate());
  REQUIRE_NOTHROW(plug->deactivate());
  REQUIRE_NOTHROW(plug->deinit());
  plug->activate();
  testIFeatureComponent(plug->getFeatureComponent());
  testIInfoComponent(plug->getInfoComponent());
  testIPortComponent(plug->getPortComponent());
  plug->deactivate();
}