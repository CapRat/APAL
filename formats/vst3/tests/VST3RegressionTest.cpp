#include "CatchTools.hpp"
#include "VST3Module.hpp"
#include "regression_data.hpp"
TEST_CASE("VST3 Midi Forwarding regression") {
    VST3Module module(MIDI_FORWARDER);
    module.initialize();
    module.allocateData(512);
    Steinberg::Vst::Event noteOnEvent;
    memset(&noteOnEvent, 0, sizeof(Steinberg::Vst::Event));
    noteOnEvent.type = noteOnEvent.kNoteOnEvent;
    noteOnEvent.noteOn = Steinberg::Vst::NoteOnEvent{ 0,22,0,1.0,0,-1 };

    Steinberg::Vst::Event noteOffEvent;
    memset(&noteOffEvent, 0, sizeof(Steinberg::Vst::Event));
    noteOffEvent.type = noteOffEvent.kNoteOffEvent;
    noteOffEvent.noteOff = Steinberg::Vst::NoteOffEvent{ 0,22,1.0,0,-1 };
    module.getDataBuffer()->inputEvents->addEvent(noteOnEvent);
    module.getDataBuffer()->inputEvents->addEvent(noteOffEvent);
    module.run();
    REQUIRE_MESSAGE(module.getDataBuffer()->outputEvents->getEventCount() == 2*2, "Error, could not forward midi.");
    Steinberg::Vst::Event e;
    module.getDataBuffer()->outputEvents->getEvent(0, e);
    INFO("Error, Wrong Midi event.")
    REQUIRE(e.type == e.kNoteOnEvent);

    module.getDataBuffer()->outputEvents->getEvent(1, e);
    REQUIRE(e.type == e.kNoteOffEvent);
    module.deinitalize();
}
