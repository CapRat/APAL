#include "CatchTools.hpp"
#include "LV2Module.hpp"
#include "regression_data.hpp"
/**
 * @brief Testcase, which tests, weather Midiforwarding is working or not.
 */
TEST_CASE("LV2 Midi Forwarding regression")
{
  LV2Module mod(MIDI_FORWARDER);
  double sampleRate = 512;
  size_t sampleCount = 512;
  for (auto plug : mod.plugins) {
    plug.instantiate(sampleRate);
    plug.allocateAndConnectPorts(sampleCount);
    plug.activate();
    for (auto& p : plug.ports) {
      if (p.type == PortType::Midi && p.dir == Direction::Input) {
        // uint8_t* x= { 0x1,0x2,0x3 };
        p.addMidiMsg(0x1, 0x2, 0x3);
        p.addMidiMsg(0xFF, 0xFF, 0xFF);
      }
    }
    plug.run(sampleRate);

    for (auto& p : plug.ports) {
      if (p.type == PortType::Midi && p.dir == Direction::Output) {
        auto data = reinterpret_cast<MidiEventBuffer*>(p.data);
        INFO("Error, could not forward Midimessages.");
        auto item1 = &data->midiEvents[0];
        REQUIRE(item1->msg[0] == 0x1);
        REQUIRE(item1->msg[1] == 0x2);
        REQUIRE(item1->msg[2] == 0x3);
        auto item2 = reinterpret_cast<MIDINoteEvent*>(
          lv2_atom_sequence_next(&data->midiEvents->event));
        REQUIRE(item2->msg[0] == 0xFF);
        REQUIRE(item2->msg[1] == 0xFF);
        REQUIRE(item2->msg[2] == 0xFF);
      }
    }
    plug.deactivate();
  }
  // REQUIRE_MESSAGE(GlobalData().getNumberOfRegisteredPlugins() >= 1, "Static
  // Initialisation Failed");
}
