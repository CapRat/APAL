/**
 * @file Implementation file for the LADSPA Format. The needed API functions are
 * defined here. Also the access to the Plugin is implemented here.
 */
#include "ladspa.h"
#include <cstring>
#include <interfaces/IPlugin.hpp>
#include <interfaces/Ports/IAudioPort.hpp>
#include <tools/PortHandling.hpp>
#include <vector>

using namespace XPlug;

struct LADSPAHandleDataType
{
  IPlugin* plug;
  const LADSPA_Descriptor* desc;
};

extern "C"
{
  /**
   * @brief entrypoint for the LADSPA-Format. In this Function an
   * LADSPA_Descriptor is created, which maps all needed Functioncalls to
   * functionality of the inernal API. Due to the Support of multiple Plugins,
   * multiple Plugins can be registered here.
   * @param index  INdex of a Plugin in the current Binary to load.
   * @return if no Plugin is available, a nullptr is returned otherwhise an
   * Instance which maps the internal API to the LADSPA-Format is returned.
   */
  const LADSPA_Descriptor* ladspa_descriptor(unsigned long index)
  {
    if (index >=
        GlobalData().getNumberOfRegisteredPlugins()) // index is obvously out of
                                                     // range, so return 0.
      return nullptr;
    PluginPtr plug = GlobalData().getPlugin(index);
    auto desc = new LADSPA_Descriptor();
    // add the needed Implementation data to the LADSPA_Descriptor
    desc->ImplementationData =
      new LADSPAHandleDataType{ GlobalData().getPlugin(index).get(), desc };
    /******************* INSTANTIATION**********************/
    desc->instantiate = [](const LADSPA_Descriptor* descriptor,
                           unsigned long) -> LADSPA_Handle {
      // because the ImplementationData and the LADSPA_Handle are the Same, we
      // can return the ImplementationData here. But we the pluginreference in
      // this Function, so we pass that and all other needed Data over the
      // ImplementationData.
      auto data =
        static_cast<LADSPAHandleDataType*>(descriptor->ImplementationData);
      data->plug->init();
      return data;
    };
    desc->activate = [](LADSPA_Handle instance) {
      auto data = static_cast<LADSPAHandleDataType*>(instance);
      data->plug->activate();
    };
    desc->deactivate = [](LADSPA_Handle instance) {
      auto data = static_cast<LADSPAHandleDataType*>(instance);
      data->plug->deactivate();
    };
    desc->cleanup = [](LADSPA_Handle instance) {
      auto data = static_cast<LADSPAHandleDataType*>(instance);
      data->plug
        ->deinit(); // is needed here, because its the opposite to instantiate.
      // free everything.
      for (size_t i = 0; i < data->desc->PortCount; i++) {
        delete[] data->desc->PortNames[i];
      }
      delete data->desc->PortNames;
      delete data->desc->PortDescriptors;
      delete data->desc;
      delete data;
    };
    // LADSPA_PROPERTY_HARD_RT_CAPABLE and other stuff, not supported yet.
    desc->Properties = 0;

    /*********************PORT HANDLING*********************/
    desc->PortCount =
      static_cast<unsigned long>(getAudioChannelCount(plug.get()));
    desc->connect_port = [](LADSPA_Handle instance,
                            unsigned long portIndex,
                            LADSPA_Data* DataLocation) {
      auto data = static_cast<LADSPAHandleDataType*>(instance);
      // portIndex is in or outputPort
      if (portIndex < getAudioChannelCount(data->plug)) {
        getAudioChannelFromIndex(data->plug, portIndex)->feed(DataLocation);
      } else {
        // Not SUpported yet
      }
    };
    // TODO: When adding Parameter, the have to be mapped to ports here.

    // Ugly allocations, but they are needed... Its just for compatiblity with
    // c.
    char** portNamesCArray = new char*[desc->PortCount * sizeof(const char*)];
    auto portDescripors =
      new LADSPA_PortDescriptor[desc->PortCount *
                                sizeof(LADSPA_PortDescriptor)];
    auto rangeHints =
      new LADSPA_PortRangeHint[desc->PortCount * sizeof(LADSPA_PortDescriptor)];
    int curIndex = 0;
    // Iterate through all Audioports and try to allocate Portname, rangehints.
    // Still needed for c compatibility. If someone has an more elegant and more
    // c++ Way, pls make a commit!
    iteratePorts<IAudioPort>(
      plug.get(),
      [&portNamesCArray, &portDescripors, &rangeHints, &curIndex](IAudioPort* p,
                                                                  size_t) {
        for (size_t i = 0; i < p->size(); i++) {
          std::string name = p->getPortName().to_string() +
                             (p->at(i)->getName() != ""
                                ? static_cast<std::string>(p->at(i)->getName())
                                : std::to_string(i));
          portNamesCArray[curIndex] = new char[name.length() + 1];
          std::strcpy(portNamesCArray[curIndex], name.c_str());
          rangeHints[curIndex] = { 0, 0, 0 };
          curIndex++;
        }
        return false;
      });

    desc->PortNames = portNamesCArray;

    desc->PortDescriptors = portDescripors;
    desc->PortRangeHints = rangeHints;

    /************************INFORMATION***************************/
    desc->Copyright = plug->getInfoComponent()->getPluginCopyright().data();
    desc->Label = nullptr;
    desc->Maker = plug->getInfoComponent()->getCreatorName().data();
    desc->Name = plug->getInfoComponent()->getPluginName().data();
    // desc->Name = "Hans peter";
    desc->UniqueID = 278375745;

    desc->run = [](LADSPA_Handle instance, unsigned long SampleCount) {
      auto data = static_cast<LADSPAHandleDataType*>(instance);
      // Set current Samplesize/Count on everyrun, because she can be different.
      // Maybe add some optimizations here.
      iteratePorts<IAudioPort>(data->plug,
                               [SampleCount](IAudioPort* p, size_t) {
                                 p->setSampleCount(SampleCount);
                                 return false;
                               });
      data->plug->processAudio();
    };

    /*    desc->run_adding = [](LADSPA_Handle instance, unsigned long
       SampleCount) { LADSPA_Descriptor* desc = (LADSPA_Descriptor*)instance;
                IPlugin* plug = (IPlugin*)desc->ImplementationData;
                plug->processAudio(plug->getPortComponent()->getInputPorts(),
       plug->getPortComponent()->getOutputPorts());
            };*/
    // desc->set_run_adding_gain;
    return desc;
  }
}
