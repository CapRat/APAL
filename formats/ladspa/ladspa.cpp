#include "ladspa.h"
#include <cstring>
#include <interfaces/IPlugin.hpp>
#include <tools/PluginUtils.hpp>
#include <vector>
using namespace XPlug;

static std::vector<LADSPA_Descriptor*> ladspaDescriptorArray;

struct LADSPAHandleDataType {
    IPlugin* plug;
    const LADSPA_Descriptor* desc;
};

extern "C" {
const LADSPA_Descriptor* ladspa_descriptor(unsigned long index)
{
    if (index >= GlobalData().getNumberOfRegisteredPlugins())
        return nullptr;
    PluginPtr plug = GlobalData().getPlugin(index);
    auto desc = new LADSPA_Descriptor();

    desc->ImplementationData = new LADSPAHandleDataType { GlobalData().getPlugin(index).get(), desc };
    /******************* INSTANTIATION**********************/
    desc->instantiate = [](const LADSPA_Descriptor* descriptor, unsigned long SampleRate) -> LADSPA_Handle {
        auto data = static_cast<LADSPAHandleDataType*>(descriptor->ImplementationData);
        data->plug->init();
        return data;
        //return NULL;
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
        data->plug->deinit();
        for (size_t i = 0; i < data->desc->PortCount; i++) {
            delete[] data->desc->PortNames[i];
        }
        delete data->desc->PortNames;
        delete data->desc->PortDescriptors;
        delete data;
    };

    desc->Properties = 0; // LADSPA_PROPERTY_HARD_RT_CAPABLE and other stuff, not supported yet.
    /*********************PORT HANDLING*********************/
    desc->PortCount = getChannelCount(plug.get());
    desc->connect_port = [](LADSPA_Handle instance, unsigned long portIndex, LADSPA_Data* DataLocation) {
        auto data = static_cast<LADSPAHandleDataType*>(instance);
        if (portIndex < getChannelCount(data->plug)) { // portIndex is in or outputPort
            getChannelFromIndex(data->plug, portIndex)->data32 = DataLocation;
        } else {
            //Not SUpported yet
        }
    };
    //TODO: When adding Parameter, the have to be mapped to ports here.

    int curIndex = 0;
    char** portNamesCArray = new char*[desc->PortCount * sizeof(const char*)];
    auto portDescripors = new LADSPA_PortDescriptor[desc->PortCount * sizeof(LADSPA_PortDescriptor)];
    auto rangeHints = new LADSPA_PortRangeHint[desc->PortCount * sizeof(LADSPA_PortDescriptor)];
    iteratePorts(plug.get(), [&portNamesCArray, &portDescripors, &rangeHints, &curIndex](Port& p, size_t portIndex) {
        for (size_t i = 0; i < p.channels.size(); i++) {
            std::string name = p.name + (p.channels[i].name != "" ? p.channels[i].name : std::to_string(i));
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
    desc->Copyright = plug->getPluginInfo()->copyright.c_str();
    desc->Label = nullptr;
    desc->Maker = plug->getPluginInfo()->creater.c_str();
    desc->Name = plug->getPluginInfo()->name.c_str();
    //desc->Name = "Hans peter";
    desc->UniqueID = 278375745;

    desc->run = [](LADSPA_Handle instance, unsigned long SampleCount) {
        auto data = static_cast<LADSPAHandleDataType*>(instance);
        data->plug->processAudio(data->plug->getPortComponent()->getInputPorts(), data->plug->getPortComponent()->getOutputPorts());
    };

    /*    desc->run_adding = [](LADSPA_Handle instance, unsigned long SampleCount) {
                LADSPA_Descriptor* desc = (LADSPA_Descriptor*)instance;
                IPlugin* plug = (IPlugin*)desc->ImplementationData;
                plug->processAudio(plug->getPortComponent()->getInputPorts(), plug->getPortComponent()->getOutputPorts());
            };*/
    desc->set_run_adding_gain;
    return desc;
}
}
