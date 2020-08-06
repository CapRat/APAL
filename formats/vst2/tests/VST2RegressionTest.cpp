#include "CatchTools.hpp"
#include "VST2Module.hpp"
#include "regression_data.hpp"
TEST_CASE("VST2 Midi Forwarding regression") {
    VST2Module module(MIDI_FORWARDER, [](AEffect* effect, int32_t opCode, int32_t index, intptr_t value , void* ptr, float)->intptr_t {
        switch (opCode) {
        case audioMasterProcessEvents: {
            auto events = (VstEvents*)ptr;
            for (int i = 0; i < events->numEvents; i++) {
                auto midiEvent = (const VstMidiEvent*)events->events[i];
                INFO("ERROR, while forwarding Midi Message.");
                REQUIRE(midiEvent->midiData[0] == 0x1);
                REQUIRE(midiEvent->midiData[1] == 0x2);
                REQUIRE(midiEvent->midiData[2] == 0x3);
            }
        }
            break;
        case audioMasterVersion:
            return 100;
 
        }
        return 0;
        });
  //  module.aMasterCallback
    module.intialise();
    module.allocate(512);
    module.run();
    uint8_t msg[3] = { 0x1,0x2,0x3 };
    module.sendMidi(msg);
    module.run();
    module.deinitialize();
    module.free();

}