#include "VolumePlugin.hpp"
REGISTER_PLUGIN(VolumePlugin);
#include <base/Ports.hpp>
VolumePlugin::VolumePlugin()
{
    this->portComponent.addPort(std::make_unique<MonoPort>("In0", PortType::Audio, PortDirection::Input));
    this->portComponent.addPort(std::make_unique<MonoPort>("Out0", PortType::Audio, PortDirection::Output));
}

void VolumePlugin::processAudio()
{
    auto in0 = dynamic_cast<IAudioPort*>( this->getPortComponent()->inputPortAt(0));
    auto out0 = dynamic_cast<IAudioPort*>(this->getPortComponent()->outputPortAt(0));
    for (int i = 0; i < in0->size(); i++) {
        IAudioChannel::AudioChannelData inData,outData;
        in0->typesafeAt(i)->get(&inData);
        in0->typesafeAt(i)->get(&outData);
        for (int s = 0; s < in0->getSampleSize(); s++) {
             inData.data32[s]=outData.data32[s] * 0.5f;
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