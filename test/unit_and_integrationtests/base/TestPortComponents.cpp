#include "CatchTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/PortComponents.hpp>
#include <base/Ports.hpp>
using namespace XPlug;

TEST_CASE("Test DynamicPortComponent")
{
  DynamicPortComponent dynComp;
  DynamicPortComponent initFillDynComp{
    std::make_shared<MonoPort>("In0", PortDirection::Input),
    std::make_shared<MonoPort>("Out0", PortDirection::Output)
  };
  DynamicPortComponent emptyDynComp;

  // testing, weather Monoport does, what it should do. If it does, continue
  // with this tests.
  auto mPort = MonoPort("In0", PortDirection::Input);
  REQUIRE_MESSAGE(
    mPort.getPortName() == "In0",
    "The Port, works not properly. No PortComponenttests can be executed");
  testIPort(&mPort);

  dynComp.addPort(std::make_unique<MonoPort>("In0", PortDirection::Input));
  dynComp.addPort(std::make_unique<MonoPort>("In1", PortDirection::Input));
  dynComp.addPort(std::make_unique<MonoPort>("In2", PortDirection::Input));
  dynComp.addPort(std::make_unique<MonoPort>("Out0", PortDirection::Output));
  dynComp.addPort(std::make_unique<MonoPort>("Out1", PortDirection::Output));
  dynComp.addPort(std::make_unique<MonoPort>("Out2", PortDirection::Output));

  dynComp.addPort(
    std::make_unique<QueueMidiPort>("MidiIn0", PortDirection::Input));
  dynComp.addPort(
    std::make_unique<QueueMidiPort>("MidiIn1", PortDirection::Input));
  dynComp.addPort(
    std::make_unique<QueueMidiPort>("MidiOut0", PortDirection::Output));

  SECTION("Testing the Interface of DynamicPortComponent")
  {
    testIPortComponent(&dynComp);
    testIPortComponent(&initFillDynComp);
    testIPortComponent(&emptyDynComp);
  }

  SECTION("Testing DynamicPortComponent functionality")
  {
    INFO("No Valid name in test");
    REQUIRE(dynComp.at(0)->getPortName() == "In0");
    REQUIRE(dynComp.at(1)->getPortName() == "In1");
    REQUIRE(dynComp.at(2)->getPortName() == "In2");
    REQUIRE(dynComp.at(3)->getPortName() == "Out0");
    REQUIRE(dynComp.at(4)->getPortName() == "Out1");
    REQUIRE(dynComp.at(5)->getPortName() == "Out2");
    REQUIRE(dynComp.at(6)->getPortName() == "MidiIn0");
    REQUIRE(dynComp.at(7)->getPortName() == "MidiIn1");
    REQUIRE(dynComp.at(8)->getPortName() == "MidiOut0");
    REQUIRE(initFillDynComp.at(0)->getPortName() == "In0");
    REQUIRE(initFillDynComp.at(1)->getPortName() == "Out0");
  }
}

TEST_CASE("Test StaticPortComponent")
{
  StaticPortComponent<6> staticComp{
    std::make_shared<MonoPort>("In0", PortDirection::Input),
    std::make_shared<MonoPort>("In1", PortDirection::Input),
    std::make_shared<MonoPort>("In2", PortDirection::Input),
    std::make_shared<MonoPort>("Out0", PortDirection::Output),
    std::make_shared<MonoPort>("Out1", PortDirection::Output),
    std::make_shared<MonoPort>("Out2", PortDirection::Output)
  };

  StaticPortComponent<0> emptyStaticComp{};
  // testing, weather Monoport does, what it should do. If it does, continue
  // with this tests.
  auto mPort = MonoPort("In0", PortDirection::Input);
  REQUIRE_MESSAGE(
    mPort.getPortName() == "In0",
    "The Port, works not properly. No PortComponenttests can be executed");
  testIPort(&mPort);

  SECTION("Testing the Interface of DynamicPortComponent")
  {
    testIPortComponent(&staticComp);
    testIPortComponent(&emptyStaticComp);
  }

  SECTION("Testing StaticPortComponent functionality")
  {
    INFO("No Valid name in test");
    REQUIRE(staticComp.at(0)->getPortName() == "In0");
    REQUIRE(staticComp.at(1)->getPortName() == "In1");
    REQUIRE(staticComp.at(2)->getPortName() == "In2");
    REQUIRE(staticComp.at(3)->getPortName() == "Out0");
    REQUIRE(staticComp.at(4)->getPortName() == "Out1");
    REQUIRE(staticComp.at(5)->getPortName() == "Out2");
  }
}