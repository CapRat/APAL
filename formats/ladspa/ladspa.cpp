#include "ladspa.h"
#include "IPlugin.hpp"
#include <vector>

static  std::vector<LADSPA_Descriptor> ladspaDescriptorArray;

void update_ports(PluginPtr plug, LADSPA_Descriptor& desc) {
	auto pinf = plug->getPluginInfo();
	desc.PortCount = pinf.numAudioInputPorts + pinf.numAudioOutputPorts + pinf.numMidiInputPorts + pinf.numMidiOutputPorts+plug->getParameterCount();

	//desc.PortDescriptors = LADSPA_PortDescriptor[desc.PortCount];
	desc.PortDescriptors = (LADSPA_PortDescriptor*)(desc.PortCount * sizeof(LADSPA_Descriptor));
	desc.PortNames=(const char *const *)(desc.PortCount * sizeof(char));
	desc.PortRangeHints= (LADSPA_PortRangeHint*)(desc.PortCount * sizeof(LADSPA_Descriptor));

	/*for (int i = 0; i < pinf.numAudioInputPorts; i++) {
		desc.PortDescriptors[i]
	}
	desc.*/

}
//Initialize a single Plugin.
void init_plugin(PluginPtr plug){
	LADSPA_Descriptor desc;
	ladspaDescriptorArray.push_back(desc);
	desc.activate= [](LADSPA_Handle instance) {};
	desc.cleanup = [](LADSPA_Handle instance) {};
	desc.connect_port = [](LADSPA_Handle instance, unsigned long Port, LADSPA_Data* DataLocation) {
	};
	desc.Copyright = plug->getPluginInfo().copyright.c_str();
	desc.deactivate= [](LADSPA_Handle instance) {};
	desc.ImplementationData = nullptr;
	desc.instantiate = [](const LADSPA_Descriptor* descriptor, unsigned long SampleRate)->LADSPA_Handle { return NULL; };
	desc.Label=NULL;
	desc.Maker = plug->getPluginInfo().creater.c_str();
	desc.Name = plug->getPluginInfo().name.c_str();

	/*desc.PortCount;
	desc.PortDescriptors;
	desc.PortNames;
	desc.PortRangeHints;*/
	update_ports(plug, desc);
	desc.Properties;
	desc.run;
	desc.run_adding;
	desc.set_run_adding_gain;
	desc.UniqueID;
}

//Initializes the plugins, if not already done.
void init_plugins() {
	if (ladspaDescriptorArray.size() != GlobalData().getNumberOfRegisteredPlugins()) {
		for (int x = 0; x < GlobalData().getNumberOfRegisteredPlugins(); x++) {
			init_plugin(GlobalData().getPlugin(x));
		}
	}
}


extern "C" {
	const LADSPA_Descriptor* ladspa_descriptor(unsigned long Index)
	{
		init_plugins(); // not the best way to initialize.... but it will work hopefully.
		if (Index < ladspaDescriptorArray.size()) {
			return &(ladspaDescriptorArray[Index]);
		}
		return nullptr;
	}
}