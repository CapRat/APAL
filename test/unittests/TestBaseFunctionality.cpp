#include <catch.hpp>
//#include <interfaces/IPlugin.hpp>
#include <GlobalData.hpp>
#include "base/LazyPlugin.hpp"
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
	REQUIRE(GlobalData().getNumberOfRegisteredPlugins() >= 1);
	INFO("Static Initialisation Works");
	
}