#ifndef EXAMPLE_PLUGIN_HPP
#define EXAMPLE_PLUGIN_HPP
#include <IPlugin.hpp>
class VolumePlugin :public IPlugin {
	// Geerbt über IPlugin
	virtual void processAudio(std::vector<audio_data> inputs, std::vector<audio_data> outputs) override;
	virtual void init() override;
	virtual void deinit() override;
	virtual void activate() override;
	virtual void deactivate() override;
	virtual PluginInfo getPluginInfo() override;
	virtual size_t getParameterCount() override;
	virtual void* getParameter() override;
	virtual void setParameter(void*) override;
	virtual std::vector<Port> getPorts() override;
};

#endif //! EXAMPLE_PLUGIN_HPP