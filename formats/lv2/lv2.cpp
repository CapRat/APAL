/**
 * @file Implementation file for the LV2 Format. The needed API functions are
 * defined here. Also the access to the Plugin is implemented here.
 */

#include "GlobalData.hpp"
#include <interfaces/IPlugin.hpp>
#include <lv2/atom/atom.h>
#include <lv2/atom/util.h>
#include <lv2/core/lv2.h>
#include <lv2/midi/midi.h>
#include <lv2/port-groups/port-groups.h>
#include <lv2/urid/urid.h>
#include <tools/PortHandling.hpp>
#include <unordered_map>
#include <vector>
using namespace XPlug;

/**
 * @brief Handle, wich is used, to create an possibility to write and read  Midi
 * more easily.
 */
struct MidiHandle
{
  LV2_Atom_Sequence* midiDataLocation;
  IMidiPort* connectedMidiPort;
  LV2_URID midi_MidiEventID;
};
/**
 * @brief Treats the MidiHandle, at it would be an input. (Put MidiMsg in Pipe)
 * @param in Input MidiHandle.
 */
inline void
handleInput(MidiHandle* in)
{
  if (in->midiDataLocation != nullptr) {
    LV2_ATOM_SEQUENCE_FOREACH(in->midiDataLocation, ev)
    {
      if (ev->body.type == in->midi_MidiEventID) {
        const uint8_t* const msg = (const uint8_t*)(ev + 1);
        in->connectedMidiPort->feed({ msg[0], msg[1], msg[2] });
      }
    }
  }
}

/**
 * @brief Struct for a 3 byte MIDI event, used for writing notes
 */
typedef struct
{
  LV2_Atom_Event event;
  uint8_t msg[3];
} MIDINoteEvent;

/**
 * @brief Treats the MidiHandle, at it would be an output. (fetches things from
 * the Pipe to the output)
 * @param out Output Midihandle.
 */
inline void
handleOutput(MidiHandle* out)
{
  if (out->midiDataLocation != nullptr) {
    const uint32_t out_capacity = out->midiDataLocation->atom.size;
    // Write an empty Sequence header to the output
    lv2_atom_sequence_clear(out->midiDataLocation);
    // out->midiDataLocation->atom.type = out->midi_MidiEventID;
    while (!out->connectedMidiPort->empty()) {
      auto msg = out->connectedMidiPort->get();
      MIDINoteEvent ev{
        { 0,
          { sizeof(MIDINoteEvent) - sizeof(LV2_Atom),
            out->midi_MidiEventID } }, // LV2_Atom_Event
        { msg[0], msg[1], msg[2] }     // new MidiMsg
      };

      lv2_atom_sequence_append_event(
        out->midiDataLocation, out_capacity, &ev.event);
    }
  }
}

/**
 * @brief LV2Handle, wich is used to pass Data arount, between LV2-API function
 * calls.
 */
struct LV2HandleDataType
{
  IPlugin* plug;                 // Reference to current Plugin
  const LV2_Descriptor* lv2Desc; // Reference to LV2 Descripor
  LV2_URID_Map* map;             // URID Map to map ids.
  std::vector<MidiHandle>
    midiHandles; // Midihandles, to use when writing to output.
};

inline bool
supportsMidi(IPlugin* plug, IPort* port)
{
  return ((plug->getFeatureComponent()->supportsFeature(Feature::MidiInput) &&
           port->getDirection() == PortDirection::Input) ||
          (plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput) &&
           port->getDirection() == PortDirection::Output));
}

/**
 * @brief Map, which maps URIs form plugins to ids, to match the internal API
 * structure.
 */
static std::unordered_map<std::string, uint32_t> URI_INDEX_MAP;
extern "C"
{
  /**
   * @brief Implementation of the lv2_descriptor entrypoint.
   * @param index index of a Plugin to load(compatible with lv2)
   * @return nullptr if index is invalid or an Pointer to the LV2_Descriptor for
   * the Plugin.
   */
  const LV2_Descriptor* lv2_descriptor(uint32_t index)
  {
    // return nullptr, if out of range.
    if (index >= GlobalData().getNumberOfRegisteredPlugins())
      return nullptr;
    PluginPtr plug = GlobalData().getPlugin(index);

    auto desc = new LV2_Descriptor();

    // get the identifaction URI for the Plugin.
    desc->URI = plug->getInfoComponent()->getPluginURI().data();
    URI_INDEX_MAP[std::string(plug->getInfoComponent()->getPluginURI())] =
      index;

    desc->activate = [](LV2_Handle instance) {
      auto data = static_cast<LV2HandleDataType*>(instance);
      data->plug->activate();
    };
    desc->deactivate = [](LV2_Handle instance) {
      auto data = static_cast<LV2HandleDataType*>(instance);
      data->plug->deactivate();
    };
    desc->connect_port = [](LV2_Handle instance,
                            uint32_t IPort,
                            void* DataLocation) {
      auto data = static_cast<LV2HandleDataType*>(instance);
      size_t midiPortIndex = 0;
      iteratePortsFlat(
        data->plug,
        [IPort, DataLocation, &data, &midiPortIndex](XPlug::IPort* p,
                                                     size_t ind) {
          if (IPort == ind) {
            auto midiPort = dynamic_cast<IMidiPort*>(p);
            if (midiPort != nullptr) {
              if (supportsMidi(data->plug, midiPort)) {
                if (data->midiHandles.capacity() <
                    getNumberOfPorts<IMidiPort>(data->plug, PortDirection::All))
                  // Resize if vector is not big enough
                  data->midiHandles.resize(getNumberOfPorts<IMidiPort>(
                    data->plug, PortDirection::All));
                // Allocate for every MIDI Port an Midihandle
                data->midiHandles[midiPortIndex] =
                  MidiHandle{ (LV2_Atom_Sequence*)DataLocation,
                              midiPort,
                              data->map->map(data->map->handle,
                                             LV2_MIDI__MidiEvent) };
              }
            } else {
              auto aPort = dynamic_cast<IAudioPort*>(p);
              aPort->at(ind - IPort)->feed((float*)DataLocation);
            }
          }
          if (dynamic_cast<IMidiPort*>(p) != nullptr)
            midiPortIndex++;
          return false;
        });
    };

    desc->instantiate = [](const LV2_Descriptor* descriptor,
                           double,
                           const char*,
                           const LV2_Feature* const* features) -> LV2_Handle {
      // map an uri to an id, to use it  with the internal API
      auto descriptorIndex = URI_INDEX_MAP[std::string(descriptor->URI)];
      GlobalData().getPlugin(descriptorIndex)->init();
      auto lv2Handle = new LV2HandleDataType{
        GlobalData().getPlugin(descriptorIndex).get(), descriptor, nullptr, {}
      };
      // Get The URID_Map feature, which is needed to use with midi.
      for (int i = 0; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_URID__map)) {
          lv2Handle->map = (LV2_URID_Map*)features[i]->data;
          break;
        }
      }
      // When the map feature is not present, we cant instantiate... Maybe skip
      // this, if its not absolutly neccessary.
      if (!lv2Handle->map) {
        return NULL;
      }
      return lv2Handle;
    };

    desc->cleanup = [](LV2_Handle instance) {
      auto data = static_cast<LV2HandleDataType*>(instance);
      URI_INDEX_MAP.erase(
        std::string(data->plug->getInfoComponent()->getPluginURI()));
      data->plug->deinit();
      delete data->lv2Desc;
      delete data;
    };

    desc->run = [](LV2_Handle instance, uint32_t SampleCount) {
      auto data = static_cast<LV2HandleDataType*>(instance);
      // TODO not nice. Maybe write a function, which does this, but is RT
      // Capable (non mem allocation and blocking ist allowed.

      iteratePorts<IAudioPort>(data->plug,
                               [SampleCount](IAudioPort* p, size_t) {
                                 p->setSampleCount(SampleCount);
                                 return false;
                               });
      // Handle Input Midi
      for (auto mHandle : data->midiHandles)
        if (mHandle.connectedMidiPort->getDirection() == PortDirection::Input)
          handleInput(&mHandle);
      // process (and also write output midi inside here, to the internal API)
      data->plug->processAudio();
      // Handle Output Midi, which was written while processing.
      for (auto mHandle : data->midiHandles)
        if (mHandle.connectedMidiPort->getDirection() == PortDirection::Output)
          handleOutput(&mHandle);
    };

    // Currently is realy no need to support extension data.
    desc->extension_data = [](const char*) -> const void* { return nullptr; };

    return desc;
  }

  /**
   * @brief Implementation of the lv2_lib_descriptor. Its pretty straight
   * forward and uses lv2_descriptor mainly.
   * @param  bundle_path is the path to a bundle.
   * @param  features An array of supported Features by the host.
   * @return new LV2_Lib_Descriptor wich can be used to fetch Plugins, which are
   * created for the internal API.
   */
  const LV2_Lib_Descriptor* lv2_lib_descriptor(const char*,
                                               const LV2_Feature* const*)
  {
    auto lDesc = new LV2_Lib_Descriptor;
    lDesc->cleanup = [](LV2_Lib_Handle) {};
    lDesc->get_plugin = [](LV2_Lib_Handle, uint32_t index) {
      return lv2_descriptor(index);
    };
    lDesc->size = sizeof(LV2_Lib_Descriptor);
    lDesc->handle = nullptr;
    return lDesc;
  }
}