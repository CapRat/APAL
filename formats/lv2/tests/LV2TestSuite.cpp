#include "IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include "lv2/core/lv2.h"
#include "lv2/urid/urid.h"
#include "lv2/midi/midi.h"
#include "lv2/atom/forge.h"
#include "lilv.h"
#include <vector>
#include <array>
#include <algorithm>
#include <map>
using namespace XPlug;

/*
template <typename T, size_t size>
struct Atom_Sequence{
    LV2_Atom_Sequence seq;
    T events[size];
} ;



std::string replaceInString(std::string strToChange, const std::string itemToReplace, const std::string substitute)
{
    while (strToChange.find(itemToReplace) != std::string::npos)
        strToChange.replace(strToChange.find(itemToReplace), 1, substitute);
    return strToChange;
}

struct LV2Features {
    std::vector<LV2_Feature*> features;
    std::vector<std::string> uridmapHandle;
    LV2_URID_Map uridmap;


    LV2Features() {
        uridmap = { &uridmapHandle,
           [](LV2_URID_Map_Handle handle, const char* uri)->LV2_URID {
               auto cHandle = (std::vector<std::string>*)handle;
               auto res = std::find(cHandle->begin(), cHandle->end(), std::string(uri));
               if (res != cHandle->end())
                   return std::distance(cHandle->begin(), res) + 1;
               cHandle->push_back(std::string(uri));
               return cHandle->size();
       } };
        uridMapFeature = { LV2_URID__map,&uridmap };
        features.push_back(&uridMapFeature);
    }

private:
    LV2_Feature uridMapFeature;
};
enum Direction{Input, Output};
enum PortType {
    Audio,
    Midi
};

// Struct for a 3 byte MIDI event, used for writing notes
typedef struct {
    LV2_Atom_Event event;
    uint8_t        msg[3];
} MIDINoteEvent;

struct MidiEventBuffer {
    LV2_Atom_Sequence seq;
    MIDINoteEvent midiEvents[40];
};


struct Port {
    const LilvPort* lilvPort=nullptr;
    void* data = nullptr;
    PortType type;
    LV2Features* feat; 


    void allocate(size_t sample_count) {

    
        switch (this->type) {
        case Midi:
            this->data=new MidiEventBuffer;
            memset(data, 0,sizeof(MidiEventBuffer));
            ((MidiEventBuffer*)this->data)->seq.atom.type = feat->uridmap.map(feat->uridmap.handle, LV2_ATOM__Sequence);
            MIDINoteEvent ev;
            ev.event.body.size = sizeof(MIDINoteEvent);
            ev.event.time.beats = 0;
            ev.event.body.type= feat->uridmap.map(feat->uridmap.handle, LV2_MIDI__MidiEvent);
            ev.msg[0] = 0xFF;
            ev.msg[1] = 0xFF;
            ev.msg[2] = 0xFF;

            lv2_atom_sequence_clear(&((MidiEventBuffer*)this->data)->seq);
            lv2_atom_sequence_append_event(&((MidiEventBuffer*)this->data)->seq, 
                sizeof(LV2_Atom_Sequence_Body) + sizeof(MIDINoteEvent[40]),(LV2_Atom_Event*)&ev );
            
            return;
        case Audio:
            data = new float[sample_count];
            return;
        }
    }
    ~Port(){
        switch (this->type) {
        case Midi:
            delete  (MidiEventBuffer*)this->data;
            return;
        case Audio:
            delete[] (float*)this->data;
            return;
        default:
            delete this->data;
        }
    }
};
struct Plugin {
    const LilvPlugin* lilvPlugin;
    std::vector<Port> ports;

};*/

#include "LV2Module.hpp"
class LV2TestSuite :public FormatTestSuiteBase {

public:
    LV2Module* module = nullptr;

    LV2TestSuite() {
    }
    ~LV2TestSuite() {
        if (this->module != nullptr)
            delete this->module;
    }
    inline  virtual void initialize(TestSuiteData _data) override {
        this->data = _data;
        this->module = new LV2Module(this->data.pluginPath);
    }

    virtual std::string getFormatName() override
    {
        return "LV2";
    }


    virtual SucceedState run() override
    {
        double sampleRate=512;
       // size_t sampleCount = 512;
        for (auto plug : this->module->plugins) {
            plug.verify();
            plug.instantiate(sampleRate);
            plug.allocateAndConnectPorts(512);
            plug.activate();
            for (auto& p : plug.ports) {
                if (p.type == PortType::Midi && p.dir == Direction::Input) {
                    // uint8_t* x= { 0x1,0x2,0x3 };
                    p.addMidiMsg(0x1, 0x2, 0x3);
                    p.addMidiMsg(0xFF, 0xFF, 0xFF);
                }
            }
            plug.run(sampleRate);
            plug.deactivate();
        }
        return TEST_SUCCEEDED;
    }
    
    virtual bool isSupported(std::string pluginPath) override
    {
        auto lib = LoadLib(pluginPath.c_str());
        if (lib == nullptr) return false;
        bool isSupp = LoadFunc<LV2_Descriptor_Function>(lib, "lv2_descriptor") != nullptr;
        UnloadLib(lib);
        return isSupp;
    }
};
REGISTER_TEST_SUITE(LV2TestSuite);