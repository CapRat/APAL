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

void VolumePlugin::registerPlugin()
{
}

PluginInfo VolumePlugin::getPluginInfo()
{
	PluginInfo inf;
	inf.name = "ExamplePlugin";
	return inf;
}

void VolumePlugin::setPluginInfo(PluginInfo inf)
{
}

void VolumePlugin::updatePluginInfo()
{
}

bool VolumePlugin::hasUI()
{
	return false;
}

void* VolumePlugin::getParameter()
{
	return nullptr;
}

void VolumePlugin::setParameter()
{
}

void VolumePlugin::activate()
{
}

void VolumePlugin::deactivate()
{
}

size_t VolumePlugin::getParameterCount()
{
	return size_t();
}
