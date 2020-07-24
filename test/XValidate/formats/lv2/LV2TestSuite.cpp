#include "../../IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include "lv2/core/lv2.h"
#include "lv2/urid/urid.h"
#include "lv2/midi/midi.h"
#include "lv2/atom/forge.h"
#include <vector>
#include <array>
#include <algorithm>
using namespace XPlug;

struct custom_test_data {
    int (*get_port_count)(size_t plugIndex);
    int (*get_port_job)(size_t plugIndex, size_t portIndex);
};

typedef struct {
    LV2_Atom_Event event;
    uint8_t msg[3];
} MIDINoteEvent;
template <typename T, size_t size>
struct Atom_Sequence{
    LV2_Atom_Sequence seq;
    T events[size];
} ;

class LV2TestSuite :public FormatTestSuiteBase {
public:

    virtual std::string getFormatName() override
    {
        return "LV2";
    }


    virtual SucceedState run() override
    {
        double sampleRate=512;
        auto lib = LoadLib(data.pluginPath.c_str());
        if (lib == nullptr) return false;
        auto lv2_descriptor_fnc = LoadFunc<LV2_Descriptor_Function>(lib, "lv2_descriptor");
        size_t index = 0;
        const LV2_Descriptor* desc = nullptr;
        std::vector<LV2_Feature*> features;
        std::vector<std::string> uridmapHandle;

        LV2_URID_Map uridmap{ &uridmapHandle,
            [](LV2_URID_Map_Handle handle, const char* uri)->LV2_URID {
                auto cHandle = (std::vector<std::string>*)handle;
                auto res= std::find(cHandle->begin(), cHandle->end(), std::string(uri));
                if (res != cHandle->end())
                    return std::distance(cHandle->begin(), res)+1;
                cHandle->push_back(std::string(uri));
                return cHandle->size();
        } };
        LV2_Feature uridMapFeature{ LV2_URID__map,&uridmap };

        features.push_back(&uridMapFeature);

        while (desc = lv2_descriptor_fnc(index++), desc != nullptr) {
            auto handle = desc->instantiate(desc, sampleRate,data.pluginPath.c_str(), features.data());
            desc->activate(handle);
            auto test_data = (custom_test_data*)desc->extension_data("urn:custom_test_data");
            auto portCount = test_data->get_port_count(index - 1);
            // TODO: hier bitte die verschiedenen Sachen implementieren. Müssen dann aber auch aus dateien gelesen werden. vieleicht workaround mit XPlug.
            std::vector<std::vector<float>> aData;
            std::vector< Atom_Sequence<MIDINoteEvent, 1>> mData;
            
            auto portData = std::vector<void *>(portCount);
            for (int i = 0; i < portCount; i++) {
                auto p_job = test_data->get_port_job(index - 1, i);
                if (p_job == 0 || p_job == 1) {

                    aData.push_back(std::vector<float>(sampleRate));
                    portData[i] = aData[aData.size()-1].data();
                }
                else {
                   /* size_t bufSize = 1024*8;
                    portData[i] = new uint8_t[bufSize];
                    auto x = (LV2_Atom_Sequence*)portData[i];
                    x->atom.size = bufSize;
                    x->atom.type = uridmap.map(uridmap.handle, LV2_ATOM__Sequence);
                    x->body.unit = uridmap.map(uridmap.handle, LV2_MIDI__MidiEvent);
                    x->body.pad = 0;*/
                    mData.push_back(Atom_Sequence<MIDINoteEvent, 1>{
                        { { (uint32_t)sizeof(Atom_Sequence<MIDINoteEvent, 1>::seq.body) + (uint32_t)sizeof(Atom_Sequence<MIDINoteEvent, 1>::events), uridmap.map(uridmap.handle, LV2_ATOM__Sequence) },
                            { uridmap.map(uridmap.handle, LV2_MIDI__MidiEvent), 0 }}, {}
                    });
   
                   portData[i] = &(mData[mData.size()-1]);
                    // Write an empty Sequence header to the output
                 //   lv2_atom_sequence_clear((LV2_Atom_Sequence*)portData[i]);
                    // Test write midi event

                    MIDINoteEvent* ev = new MIDINoteEvent;
                    // Could simply do fifth.event = *ev here instead...
                    ev->event.time.frames = 0;  // Same time
                    ev->event.body.type = uridmap.map(uridmap.handle, LV2_MIDI__MidiEvent);    // Same type
                    ev->event.body.size = sizeof(MIDINoteEvent);    // Same size
                    ev->msg[0] = 0x1;
                    ev->msg[1] = 0x2;
                    ev->msg[2] = 0x3;
                    lv2_atom_sequence_append_event((LV2_Atom_Sequence*)portData[i], sizeof(MIDINoteEvent)*1, &ev->event);

                }
                desc->connect_port(handle, i, portData[i]);
            }

            desc->run(handle, sampleRate);
            for (int i = 0; i < portCount; i++) {
                auto p_job = test_data->get_port_job(index - 1, i);
                if (p_job==0 || p_job==1) {
                   // delete[] (float*)portData[i];
                }
                else {
                  //  lv2_atom_sequence_clear((LV2_Atom_Sequence*)portData[i]);
                  // delete ((LV2_Atom_Sequence*)portData[i]);
                }
            }
            desc->deactivate(handle);
            desc->cleanup(handle);
        }

        return TEST_SUCCEEDED;
    }
    virtual SucceedState runPerformance() override
    {
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