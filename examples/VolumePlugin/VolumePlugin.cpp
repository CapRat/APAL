#include "VolumePlugin.hpp"
REGISTER_PLUGIN(VolumePlugin);

void VolumePlugin::processAudio(std::vector<audio_data> inputs, std::vector<audio_data> outputs)
{
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

PluginInfo VolumePlugin::getPluginInfo()
{
	return PluginInfo();
}

size_t VolumePlugin::getParameterCount()
{
	return size_t();
}

void* VolumePlugin::getParameter()
{
	return nullptr;
}

void VolumePlugin::setParameter(void*)
{
}

std::vector<Port> VolumePlugin::getPorts()
{
	return std::vector<Port>();
}
