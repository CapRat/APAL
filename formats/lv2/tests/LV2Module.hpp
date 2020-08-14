/**
 * @file Multiple classes ans structs, which should represent an LV2_Module, to
 * make hosting with tests easier. This classes are not written with that much
 * efford, because its just for testing. So if more stability is needed, Work
 * iss needed to make it more stable.
 */

#ifndef LV2_MODULE_HPP
#define LV2_MODULE_HPP

#include "lilv.h"
#include "lv2/atom/forge.h"
#include "lv2/core/lv2.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"
#include <algorithm>
#include <array>
#include <exception>
#include <map>
#include <vector>
/**
 * @brief Replaces an item in a string with a subtitude.
 * @param strToChange String to manipulate, the changed string is returned. The
 * given String is notchanged!
 * @param itemToReplace The item, which should be replaced in the strToChange,
 * if found.
 * @param substitute Value to replace the found items with.
 * @return The new changed string.
 */
inline std::string
replaceInString(std::string strToChange,
                const std::string& itemToReplace,
                const std::string& substitute)
{
  while (strToChange.find(itemToReplace) != std::string::npos)
    strToChange.replace(strToChange.find(itemToReplace), 1, substitute);
  return strToChange;
}
/**
 * @brief Struct, which holds LV2Fetures, whcih can be passed to LV2-Plugins.
 * Currently only supports LV2_URID_Map
 */
struct LV2Features
{
  std::vector<LV2_Feature*> features;
  std::vector<std::string> uridmapHandle;
  LV2_URID_Map uridmap;
  /**
   * @brief Constructor, which should construct features.
   * @return
   */
  inline LV2Features()
  {
    uridmap = { &uridmapHandle,
                [](LV2_URID_Map_Handle handle, const char* uri) -> LV2_URID {
                  auto cHandle = (std::vector<std::string>*)handle;
                  auto res = std::find(
                    cHandle->begin(), cHandle->end(), std::string(uri));
                  if (res != cHandle->end())
                    return std::distance(cHandle->begin(), res) + 1;
                  cHandle->push_back(std::string(uri));
                  return cHandle->size();
                } };
    uridMapFeature = { LV2_URID__map, &uridmap };
    features.push_back(&uridMapFeature);
  }

private:
  LV2_Feature uridMapFeature;
};
/**
 * @brief Enum, which represents a Direction
 */
enum Direction
{
  Input,
  Output
};
/**
 * @brief Enum, which represents a Porttype
 */
enum PortType
{
  Undefined,
  Audio,
  Midi
};

/**
 * @brief Struct for a 3 byte MIDI event, used for writing notes
 */
typedef struct
{
  LV2_Atom_Event event;
  uint8_t msg[3];
} MIDINoteEvent;

/**
 * @brief MidiEventBuffer Bufferstruct, which can be used as Atom Sequence and
 * can store more than 30 MidiEvents.
 */
struct MidiEventBuffer
{
  LV2_Atom_Sequence seq;
  MIDINoteEvent
    midiEvents[40]; // Chust a buffersize, The type doesnt matter. Because of
                    // the strange LV2 Padding, its not enough space for 40
                    // Events, but for enough to operate properly.
};

/**
 * @brief Port Class, which saves the given Data, and also deltes it, if not
 * used anymore. Can be used as audio and midi port. But do not change the Type
 * in between.
 */
struct Port
{
private:
  const LV2Features* features;
  size_t size_of_data = 0;

public:
  const LilvPort* lilvPort = nullptr;
  void* data = nullptr;
  PortType type;
  Direction dir;
  Port(const LilvPort* _lilvPort,
       PortType _type,
       Direction _dir,
       const LV2Features* _features)
  {
    this->lilvPort = _lilvPort;
    this->type = _type;
    this->features = _features;
    this->dir = _dir;
  }
  /**
   * @brief Speacial CopyConstructor, which memcopys data, wehen copied.
   */
  Port(const Port& other)
  {
    features = other.features;
    this->dir = other.dir;
    this->type = other.type;
    this->size_of_data = other.size_of_data;
    this->lilvPort = other.lilvPort;
    this->data = malloc(size_of_data);
    memcpy(this->data, other.data, this->size_of_data);
  }

  inline ~Port() { free(); }

  /**
   * @brief Allocates Data for an Port, with the given Samplecound.
   * If data is already allocated, it will be freed before new allocation!
   * @param sample_count Size of the new allocated Array in this Port. Only
   * Used, when this is an Audioport.
   */
  inline void allocate(size_t sample_count)
  {
    if (this->data != nullptr)
      this->free();
    switch (this->type) {
      case Midi:
        this->size_of_data = sizeof(MidiEventBuffer);
        this->data = new MidiEventBuffer;
        memset(data, 0, sizeof(MidiEventBuffer));
        reinterpret_cast<MidiEventBuffer*>(this->data)->seq.atom.type =
          features->uridmap.map(features->uridmap.handle, LV2_ATOM__Sequence);
        reinterpret_cast<MidiEventBuffer*>(this->data)->seq.atom.size =
          this->size_of_data - sizeof(LV2_Atom);
        if (this->dir == Input)
          lv2_atom_sequence_clear(
            &reinterpret_cast<MidiEventBuffer*>(this->data)->seq);
        return;
      case Audio:
        this->size_of_data = sizeof(float) * sample_count;
        data = new float[sample_count];
        return;
      default:
        return;
    }
  }

  /**
   * @brief Adds a MidiMessage to the current Buffer, only works if this is an
   * Midi Type port.
   * @param b1 first Byte of Message.
   * @param b2 second Byte of Message.
   * @param b3 third Byte of Message.
   */
  inline void addMidiMsg(uint8_t b1, uint8_t b2, uint8_t b3)
  {
    MIDINoteEvent ev{ { 0,
                        { sizeof(MIDINoteEvent) - sizeof(LV2_Atom),
                          features->uridmap.map(features->uridmap.handle,
                                                LV2_MIDI__MidiEvent) } },
                      { b1, b2, b3 } };
    lv2_atom_sequence_append_event(
      &reinterpret_cast<MidiEventBuffer*>(this->data)->seq,
      sizeof(MIDINoteEvent[40]),
      (LV2_Atom_Event*)&ev);
  }

  /**
   * @brief frees the current allocated Memory in this class. Frees nothing, if
   * nothing is allocated.
   */
  inline void free()
  {
    switch (this->type) {
      case Midi:
        delete reinterpret_cast<MidiEventBuffer*>(this->data);
        return;
      case Audio:
        delete[] reinterpret_cast<float*>(this->data);
        return;
      default:
        // delete this->data;
        return;
    }
    this->data = nullptr;
  }
};

/**
 * @brief Represents an LV2Plugin as a class.
 */
struct Plugin
{
private:
  const LV2Features* features;

public:
  LilvInstance* lilvInstance = nullptr;
  const LilvPlugin* lilvPlugin = nullptr;
  std::vector<Port> ports;
  size_t sampleRate = 0;
  /**
   * @brief Creates an LV2 PLugin representation
   * @param lilvPlugin
   * @param features
   * @param w the lilv world, where the Plugin lives in.
   */
  Plugin(const LilvPlugin* _lilvPlugin,
         const LV2Features* _features,
         LilvWorld* w)
  {
    this->lilvPlugin = _lilvPlugin;
    this->features = _features;

    auto auPort = lilv_new_uri(w, LILV_URI_AUDIO_PORT);
    auto midiEvent = lilv_new_uri(w, LILV_URI_MIDI_EVENT);
    auto inPort = lilv_new_uri(w, LILV_URI_INPUT_PORT);
    auto outPort = lilv_new_uri(w, LILV_URI_OUTPUT_PORT);
    // create the needed Ports for this Plugin.
    for (uint32_t i = 0; i < lilv_plugin_get_num_ports(this->lilvPlugin); i++) {
      auto lilvPort = lilv_plugin_get_port_by_index(this->lilvPlugin, i);
      auto pType = PortType::Undefined;
      if (lilv_port_is_a(this->lilvPlugin, lilvPort, auPort))
        pType = Audio;
      else if (lilv_port_supports_event(this->lilvPlugin, lilvPort, midiEvent))
        pType = Midi;
      auto pDir = lilv_port_is_a(this->lilvPlugin, lilvPort, inPort)
                    ? Direction::Input
                    : Direction::Output;

      this->ports.push_back(Port(lilvPort, pType, pDir, features));
    }

    lilv_node_free(auPort);
    lilv_node_free(midiEvent);
    lilv_node_free(inPort);
    lilv_node_free(outPort);
  }
  /**
   * @brief Initializes the Plugininstance. Does not activate the plugin! Must
   * be called before everything else, except verify.
   * @param sampleRate
   * @return
   */
  inline void instantiate(size_t _sampleRate)
  {
    this->sampleRate = _sampleRate;
    lilvInstance = lilv_plugin_instantiate(
      this->lilvPlugin, this->sampleRate, features->features.data());
  }

  /**
   * @brief Allocates memory for the ports, and connects them with the Plugin.
   * @param audioPortBufferSize Memorysize of each audiobuffer in every Port.
   */
  inline void allocateAndConnectPorts(size_t audioPortBufferSize)
  {
    for (size_t i = 0; i < this->ports.size(); i++) {
      this->ports[i].allocate(audioPortBufferSize);
      lilv_instance_connect_port(lilvInstance, i, this->ports[i].data);
    }
  }

  ~Plugin()
  {
    if (this->lilvInstance != nullptr)
      lilv_instance_free(lilvInstance);
  }

  inline void activate() { lilv_instance_activate(lilvInstance); }
  inline void deactivate() { lilv_instance_deactivate(lilvInstance); }
  inline void run(uint32_t sampleCount)
  {
    lilv_instance_run(lilvInstance, sampleCount);
  }
  inline void verify() { lilv_plugin_verify(this->lilvPlugin); }
};

/**
 * @brief This is the LV2Moduel class which... mainly does sth on Construction
 * and holds a Vector of Plugins, which should be used then...
 * Like i Said, if this is used more and needs to be better... there is a lot of
 * Room for improvement.
 */
class LV2Module
{
private:
  LV2Features features;
  LilvWorld* world = nullptr;

public:
  std::vector<Plugin> plugins;
  inline explicit LV2Module(const std::string& pathToPlugin)
  {
    this->world = lilv_world_new();

    // Use the path of the lv2 binary. Has to have manifest.ttl in that folder,
    // to work.
    auto bundlePath = replaceInString(pathToPlugin, "\\", "/");
    if (bundlePath.substr(bundlePath.find_last_of('/') + 1, bundlePath.size())
          .find('.') != std::string::npos)
      bundlePath = bundlePath.substr(0, bundlePath.find_last_of('/') + 1);

    LilvNode* bundle = lilv_new_file_uri(this->world, NULL, bundlePath.c_str());
    lilv_world_load_bundle(this->world, bundle);
    lilv_world_load_specifications(this->world);
    lilv_world_load_plugin_classes(this->world);
    auto lilvPlugins = lilv_world_get_all_plugins(this->world);
    LILV_FOREACH(plugins, plugIter, lilvPlugins)
    {
      this->plugins.push_back(Plugin(
        lilv_plugins_get(lilvPlugins, plugIter), &this->features, this->world));
    }
    lilv_node_free(bundle);
  }

  inline ~LV2Module() { lilv_world_free(this->world); }
};

#endif //! LV2_MODULE_HPP