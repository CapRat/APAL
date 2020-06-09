#include "base/LazyPlugin.hpp"
using namespace XPlug;
class  SimpleExamplePlugin : public LazyPlugin {
public:

	// Geerbt über LazyPlugin
	virtual void processAudio(const std::vector<Port>& inputs,  std::vector<Port>& outputs) override
	{
	}

};

REGISTER_PLUGIN(SimpleExamplePlugin);