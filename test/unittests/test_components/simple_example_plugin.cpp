#include "IPlugin.hpp"

class  SimpleExamplePlugin : public IPlugin {
public:
	

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

	virtual PluginInfo getPluginInfo() override
	{
		return PluginInfo();
	}

	virtual size_t getParameterCount() override
	{
		return size_t();
	}

	virtual void* getParameter() override
	{
		return nullptr;
	}

	virtual void setParameter(void*) override
	{
	}

	virtual std::vector<Port> getPorts() override
	{
		return std::vector<Port>();
	}

};

REGISTER_PLUGIN(SimpleExamplePlugin);