#include "catch2/catch.hpp"
//#include <interfaces/IPlugin.hpp>
#include <GlobalData.hpp>
using namespace XPlug;
TEST_CASE("Registration of derived Plugins"){
	REQUIRE(GlobalData().getNumberOfRegisteredPlugins() == 1);
	INFO("Static Initialisation Works");
	
}