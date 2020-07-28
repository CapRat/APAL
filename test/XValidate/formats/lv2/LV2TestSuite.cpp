#include "../../IFormatTestSuite.hpp"
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

struct custom_test_data {
    int (*get_port_count)(size_t plugIndex);
    int (*get_port_job)(size_t plugIndex, size_t portIndex);
};

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
    /**
     * @brief Constructor, which should construct features.
     * @return 
    */
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

/**
 * @brief Port Class, which saves the given Data, and also deltes it, if not used anymore.
*/
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

};

class LV2TestSuite :public FormatTestSuiteBase {

public:
    LV2Features features;
    LilvWorld* world=nullptr;
    std::vector<Plugin> plugins;

    LV2TestSuite() {
    }
    ~LV2TestSuite() {
        lilv_world_free(this->world);
    }
    inline  virtual void initialize(TestSuiteData data) override {
        this->data = data;
        this->world = lilv_world_new();

        // Use the path of the lv2 binary. Has to have manifest.ttl in that folder, to work.
        auto bundlePath = replaceInString(this->data.pluginPath, "\\", "/");
        if(bundlePath.substr(bundlePath.find_last_of('/')+1, bundlePath.size()).find('.')!=std::string::npos)
            bundlePath = bundlePath.substr(0, bundlePath.find_last_of('/') + 1);

        LilvNode* bundle = lilv_new_file_uri(this->world, NULL, bundlePath.c_str());
        lilv_world_load_bundle(this->world, bundle);
        lilv_world_load_specifications(this->world);
        lilv_world_load_plugin_classes(this->world);
        auto lilvPlugins = lilv_world_get_all_plugins(this->world);

        auto auPort = lilv_new_uri(this->world, LILV_URI_AUDIO_PORT);
        auto midiEvent = lilv_new_uri(this->world, LILV_URI_MIDI_EVENT);
  
        LILV_FOREACH(plugins, plugIter, lilvPlugins) {
            Plugin p;
            p.lilvPlugin = lilv_plugins_get(lilvPlugins, plugIter);
            for (auto i = 0; i < lilv_plugin_get_num_ports(p.lilvPlugin); i++) {
                Port port;
                port.lilvPort= lilv_plugin_get_port_by_index(p.lilvPlugin, i);
                port.feat = &features;
                if (lilv_port_is_a(p.lilvPlugin, port.lilvPort, auPort))
                    port.type = Audio;
                else if (lilv_port_supports_event(p.lilvPlugin, port.lilvPort, midiEvent))
                    port.type = Midi;
                p.ports.push_back(port);
            }
            this->plugins.push_back(p);
        }

        lilv_node_free(auPort);
        lilv_node_free(midiEvent);
    }

    virtual std::string getFormatName() override
    {
        return "LV2";
    }


    virtual SucceedState run() override
    {
        double sampleRate=512;
        size_t sampleCount = 512;
        const LV2_Descriptor* desc = nullptr;
     
        for (auto plug : this->plugins) {
            lilv_plugin_verify(plug.lilvPlugin);
            auto lInstance = lilv_plugin_instantiate(plug.lilvPlugin, sampleRate, features.features.data());
            lilv_instance_activate(lInstance);
           
            for (int i = 0; i < plug.ports.size();i++) {
                plug.ports[i].allocate(sampleCount);
                lilv_instance_connect_port(lInstance, i, plug.ports[i].data);
            }
            lilv_instance_run(lInstance, sampleCount);

            lilv_instance_deactivate(lInstance);
            lilv_instance_free(lInstance);
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