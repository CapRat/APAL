#include "IPlugin.hpp"

class  SimpleExamplePlugin : public IPlugin {
public:
	SimpleExamplePlugin() = default;
	// Geerbt über IPlugin
	virtual void processAudio(std::vector<audio_data> inputs, std::vector<audio_data> outputs) override
	{
	}
	virtual void init() override
	{
	}
	virtual void deinit() override
	{
	}
	virtual void activate() override
	{
	}
	virtual void deactivate() override
	{
	}
	virtual void registerPlugin() override
	{
	}
	virtual PluginInfo getPluginInfo() override
	{
		return PluginInfo();
	}
	virtual void setPluginInfo(PluginInfo inf) override
	{
	}
	virtual void updatePluginInfo() override
	{
	}
	virtual bool hasUI() override
	{
		return false;
	}
	virtual size_t getParameterCount() override
	{
		return size_t();
	}
	virtual void* getParameter() override
	{
		return nullptr;
	}
	virtual void setParameter() override
	{
	}
};

REGISTER_PLUGIN(SimpleExamplePlugin);