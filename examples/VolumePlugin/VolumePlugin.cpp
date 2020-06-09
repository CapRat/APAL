#include "VolumePlugin.hpp"
REGISTER_PLUGIN(VolumePlugin);

VolumePlugin::VolumePlugin() : LazyPlugin({ Port{"In0" ,PortType::Audio,PortDirection::Input ,0,{Channel{0,"Channel nr1",NULL,NULL} }} ,
                                            Port{"Out0",PortType::Audio,PortDirection::Output,0,{Channel{0,"Channel nr1",NULL,NULL} }}
    })
{
}

void VolumePlugin::processAudio(const std::vector<Port>& inputs, std::vector<Port>& outputs)
{
    for (int i = 0; i < inputs[0].channels.size(); i++) {
        for (int s = 0; s < inputs[0].sampleSize; s++) {
           // outputs[0].channels[i].data32[s] = inputs[0].channels[i].data32[s];
           // outputs[0].channels[i].data64[s] = inputs[0].channels[i].data64[s];
            outputs[0].channels[i].data32[s] = inputs[0].channels[i].data32[s]*0.5;
           /* if (outputs[0].channels[i].data64 != nullptr) {
                outputs[0].channels[i].data64[s] = inputs[0].channels[i].data64[s] * 0.5;
            }*/
        }
    }

 /*   for (int i = 0; i < inputs[0].channels.size(); i++) {
        outputs[0].channels[i].data32 = inputs[0].channels[i].data32;
        outputs[0].channels[i].data64 = inputs[0].channels[i].data64;
    }*/
     
}

void VolumePlugin::init()
{
}

void VolumePlugin::deinit()
{
}

void VolumePlugin::activate()
{
}

void VolumePlugin::deactivate()
{
}
/*
PluginInfo* VolumePlugin::getPluginInfo()
{
	return PluginInfo();
}

*/