#include "catch2/catch.hpp"
#include <IPlugin.hpp>
#include <GlobalData.hpp>

TEST_CASE("Registration of derived Plugins"){
	REQUIRE(GlobalData().getNumberOfRegisteredPlugins() == 1);
	INFO("Static Initialisation Works");
	
}