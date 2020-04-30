#include "vestige.h"
extern "C" {
	const AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
		return nullptr;
	}
	//const AEffect* main(audioMasterCallback audioMaster) { return VSTPluginMain(audioMaster); }

}