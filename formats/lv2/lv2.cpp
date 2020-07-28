#include <lv2/core/lv2.h>
#include <lv2/port-groups/port-groups.h>
#include <lv2/midi/midi.h>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/urid/urid.h>
#include <interfaces/IPlugin.hpp>
#include "GlobalData.hpp"
#include <tools/PortHandling.hpp>
#include <vector>
#include <unordered_map>
using namespace XPlug;
struct custom_test_data {
    int (*get_port_count)(size_t plugIndex);
    int (*get_port_job)(size_t plugIndex,size_t portIndex);
};


struct MidiHandle {
    LV2_Atom_Sequence* midiDataLocation;
    IMidiPort* connectedMidiPort;
    LV2_URID midi_MidiEventID;
    
};
/**
 * @brief Treats the MidiHandle, at it would be an input. (Put MidiMsg in Pipe)
 * @param in 
*/
inline void handleInput(MidiHandle* in) {
    if (in->midiDataLocation != nullptr) {
        LV2_ATOM_SEQUENCE_FOREACH(in->midiDataLocation, ev) {
            if (ev->body.type == in->midi_MidiEventID) {
                const uint8_t* const msg = (const uint8_t*)(ev + 1);
                in->connectedMidiPort->feed({ msg[0],msg[1],msg[2] });
            }
        }
    }
}

// Struct for a 3 byte MIDI event, used for writing notes
typedef struct {
    LV2_Atom_Event event;
    MidiMessage msg;
} MIDINoteEvent;
/**
 * @brief Treats the MidiHandle, at it would be an output. (fetches things from the Pipe to the output)
 * @param in
*/
inline void handleOutput(MidiHandle* out) {
    if (out->midiDataLocation != nullptr) {
        const uint32_t out_capacity = out->midiDataLocation->atom.size;
        // Write an empty Sequence header to the output
        lv2_atom_sequence_clear(out->midiDataLocation);
        out->midiDataLocation->atom.type = out->midi_MidiEventID;
        while (!out->connectedMidiPort->empty()) {
            MIDINoteEvent ev;
            // Could simply do fifth.event = *ev here instead...
            ev.event.time.frames = 0;  // Same time
            ev.event.body.type = out->midi_MidiEventID;    // Same type
            ev.event.body.size = sizeof(MIDINoteEvent);    // Same size
            ev.msg = out->connectedMidiPort->get();
            lv2_atom_sequence_append_event(
                out->midiDataLocation, out_capacity, &ev.event);
        }
    }
}


struct LV2HandleDataType {
    IPlugin* plug;
    const LV2_Descriptor* lv2Desc;
    LV2_URID_Map* map;
    std::vector<MidiHandle> midiHandles;
};

inline bool supportsMidi(IPlugin* plug,IPort* port) {
    return((plug->getFeatureComponent()->supportsFeature(Feature::MidiInput) && port->getDirection() == PortDirection::Input) ||
        (plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput) && port->getDirection() == PortDirection::Output));
}

/**
   The `lv2_descriptor()` function is the entry point to the plugin library.  The
   host will load symbols the library and call this function repeatedly with increasing
   indices to find all the plugins defined in the library.  The index is not an
   indentifier, the URI of the returned descriptor is used to determine the
   identify of the plugin.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
static std::unordered_map<std::string, uint32_t>  URI_INDEX_MAP;
extern "C" {
    static int currentLV2Index = 0;
    const LV2_Descriptor* lv2_descriptor(uint32_t index)
    {
        if (index < 0 || index >= GlobalData().getNumberOfRegisteredPlugins())
            return NULL;
        PluginPtr plug = GlobalData().getPlugin(index);

        auto desc = new LV2_Descriptor();

        desc->URI=plug->getInfoComponent()->getPluginURI().data();
        URI_INDEX_MAP[std::string(plug->getInfoComponent()->getPluginURI())]= index;

        //LV2_Descriptor desc = lv2DescriptorArray.at(lv2DescriptorArray.size() - 1);
        desc->activate = [](LV2_Handle instance) {
            auto data = static_cast<LV2HandleDataType*>(instance);
            data->plug->activate();
        };
        desc->deactivate = [](LV2_Handle instance) {
            auto data = static_cast<LV2HandleDataType*>(instance); 
            data->plug->deactivate();
        };
        desc->connect_port = [](LV2_Handle instance, uint32_t IPort, void* DataLocation) {
            auto data = static_cast<LV2HandleDataType*>(instance);
            size_t midiPortIndex = 0;
            iteratePortsFlat(data->plug, [IPort,DataLocation,&data,&midiPortIndex](XPlug::IPort* p, size_t ind) {
                if (IPort == ind) {
                    auto midiPort = dynamic_cast<IMidiPort*>(p);
                    if (midiPort != nullptr)
                        if( supportsMidi(data->plug,midiPort)) {
                            if (data->midiHandles.capacity() < getNumberOfPorts<IMidiPort>(data->plug,PortDirection::All)) // Resize if vector is not big enough
                                data->midiHandles.resize(getNumberOfPorts<IMidiPort>(data->plug, PortDirection::All));
                        data->midiHandles[midiPortIndex] = MidiHandle{ (LV2_Atom_Sequence*)DataLocation,midiPort, data->map->map(data->map->handle, LV2_MIDI__MidiEvent) };
                    }
                    else {
                        auto aPort = dynamic_cast<IAudioPort*>(p);
                        aPort->at(ind - IPort)->feed((float*)DataLocation);
                    }
   
                }
                if (dynamic_cast<IMidiPort*>(p) != nullptr)
                    midiPortIndex++;
                return false;
                });

         /*  auto midiPort = dynamic_cast<IMidiPort*>(data->plug->getPortComponent()->at(IPort));
            if (midiPort != nullptr && (
                (data->plug->getFeatureComponent()->supportsFeature(Feature::MidiInput)&&midiPort->getDirection()==PortDirection::Input ) ||
                (data->plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput) && midiPort->getDirection() == PortDirection::Output)
                )) {
                 data->midiHandles.push_back( MidiHandle{
                    (LV2_Atom_Sequence*)DataLocation,
                    midiPort,
                    data->map->map(data->map->handle, LV2_MIDI__MidiEvent)
                });
            }
            else {
                getAudioChannelFromIndex(data->plug, IPort)->feed((float*)DataLocation);
            }*/
        //  midiData->body.
        };
    
        desc->instantiate = [](const LV2_Descriptor* descriptor, double SampleRate, const char* bundlePath, const LV2_Feature* const* features)->LV2_Handle {
            auto index = URI_INDEX_MAP[std::string(descriptor->URI)];
            GlobalData().getPlugin(index)->init();
            auto lv2Handle = new LV2HandleDataType{ GlobalData().getPlugin(index).get(),descriptor };
            for (int i = 0; features[i]; ++i) {
                if (!strcmp(features[i]->URI, LV2_URID__map)) {
                    lv2Handle->map = (LV2_URID_Map*)features[i]->data;
                    break;
                }
            }
            if (!lv2Handle->map) {
                return NULL;
            } 
            return lv2Handle;
        };

        desc->cleanup = [](LV2_Handle instance) {
            auto data =static_cast<LV2HandleDataType *>( instance);
            URI_INDEX_MAP.erase(std::string(data->plug->getInfoComponent()->getPluginURI()));
            data->plug->deinit();
            delete data->lv2Desc;
            delete data;
        };

        desc->run = [](LV2_Handle instance, uint32_t SampleCount) {
            auto data = static_cast<LV2HandleDataType*>(instance); 
            //TODO not nice. Maybe write a function, which does this, but is RT Capable (non mem allocation and blocking ist allowed.
            
            iteratePorts<IAudioPort>(data->plug, [SampleCount](IAudioPort* p, size_t index) {
                p->setSampleSize(SampleCount);
                return false;
                });
            for (auto mHandle : data->midiHandles)
                if(mHandle.connectedMidiPort->getDirection()==PortDirection::Input)
                    handleInput(&mHandle);
            data->plug->processAudio();
            for (auto mHandle : data->midiHandles)
                if (mHandle.connectedMidiPort->getDirection() == PortDirection::Output)
                handleOutput(&mHandle);
        };

        desc->extension_data = [](const char* uri)-> const void* {
            return nullptr;
        };

        return desc;
    }

    const LV2_Lib_Descriptor* lv2_lib_descriptor(const char* bundle_path, const LV2_Feature* const* features)
    {
        auto lDesc= new LV2_Lib_Descriptor;
        lDesc->cleanup = [](LV2_Lib_Handle h) {};
        lDesc->get_plugin = [](LV2_Lib_Handle h, uint32_t index) {
            return lv2_descriptor(index);
        };
        lDesc->size = sizeof(LV2_Lib_Descriptor);
        lDesc->handle = nullptr;
        return lDesc;
    }
}