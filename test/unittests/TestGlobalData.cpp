//#include <interfaces/IPlugin.hpp>
#include "UnitTools.hpp"
#include <GlobalData.hpp>
#include "base/PluginBases.hpp"
using namespace XPlug;
class  SimpleExamplePlugin : public LazyPlugin {
public:

	// Geerbt über LazyPlugin
	virtual void processAudio() override
	{
	}

};

REGISTER_PLUGIN(SimpleExamplePlugin);

using namespace XPlug;
TEST_CASE("Registration of derived Plugins"){
	REQUIRE_MESSAGE(GlobalData().getNumberOfRegisteredPlugins() >= 1,"Static Initialisation Failed");
}