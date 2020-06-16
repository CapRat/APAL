#include <catch.hpp>
#include <GlobalData.hpp>
#include <interfaces/IPlugin.hpp>
#include "base/LazyPlugin.hpp"
#include <base/Ports.hpp>
#include "UnitTools.hpp"
using namespace XPlug;

class  TestPortCompPlug : public LazyPlugin {
public:
	TestPortCompPlug() {
		this->portComponent.addPort(std::make_unique<MonoPort>("In0", PortType::Audio, PortDirection::Input));
		this->portComponent.addPort(std::make_unique<MonoPort>("Out0", PortType::Audio, PortDirection::Output));
	}
	// Geerbt über LazyPlugin
	virtual void processAudio() override
	{
	}
	virtual PluginInfo* getPluginInfo() override
	{
		static PluginInfo* inf = new PluginInfo{ "TestPortCompPlug" ,"","","","", false };
		return inf;
	}
};
REGISTER_PLUGIN(TestPortCompPlug);


PluginPtr getTestPortCompPlug() {
	for (int i = 0; i < GlobalData().getNumberOfRegisteredPlugins(); i++) {
		auto plug = GlobalData().getPlugin(i);
		if (plug->getPluginInfo()->name == "TestPortCompPlug")
			return plug;
	}
	return nullptr;
}

//TODO: create extra class for this test, which is registrated at index.
TEST_CASE("test changing of portcomponent values") {
	auto plug = getTestPortCompPlug();
	REQUIRE_MESSAGE(plug != nullptr,"Error, cant initialize Testplugin");
	float testData[] = { 0.1f,0.2f,0.3f };
	IAudioChannel::AudioChannelData d;
	static_cast<IAudioChannel*>(plug->getPortComponent()->at(0)->at(0))->feed({ testData,nullptr });
	static_cast<IAudioChannel*>(plug->getPortComponent()->at(0)->at(0))->get(&d);
	REQUIRE_MESSAGE((d.data32 == testData), "Error, cant change Values of Channels. Maybe there are reference Problems? Make sure correct refferences are passed.");
	INFO("Static Initialisation Works");

}