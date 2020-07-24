#include "UnitTools.hpp"
#include "interfaces/InterfaceTestMethods.hpp"
#include <base/Ports.hpp>
using namespace XPlug;

//function to compare array elements
char compareMidiMessage(MidiMessage a, MidiMessage b) {
    int i;
    for (i = 0; i < sizeof(MidiMessage); i++) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

TEST_CASE("Test Ports") {
    MonoPort mP{ "testMono",PortDirection::Input };
    StereoPort sP{ "testStereo", PortDirection::Input };
    Surround5_1Port surP{ "testSurr51", PortDirection::Input };
    QueueMidiPort mPort{ "testMidi", PortDirection::Input };

    SECTION("Interface tests for Ports") {
        testIPort(&mP);
        testIPort(&sP);
        testIPort(&surP);
        testIPort(&mPort);
    }

    REQUIRE_MESSAGE(mP.size() == 1, "Mono should have 1 Channel");
    REQUIRE_MESSAGE(sP.size() == 2, "Stereo should have 2 Channel");
    REQUIRE_MESSAGE(surP.size() == 6, "5.1 should have 6 Channel");


    REQUIRE_MESSAGE(mPort.size() == 0, "No Messages where feeded");
    
    mPort.feed({ 0x1,0x2,0x3 });
    mPort.feed({ 0x2,0x3,0x4 });
    REQUIRE_MESSAGE(compareMidiMessage(mPort.peek(), { 0x1, 0x2, 0x3 }), "Queue not working properly");
    REQUIRE_MESSAGE(compareMidiMessage(mPort.get(), { 0x1, 0x2, 0x3 }), "Queue not working properly");
    REQUIRE_MESSAGE(compareMidiMessage(mPort.get(), { 0x2, 0x3, 0x4 }), "Queue not working properly");
}