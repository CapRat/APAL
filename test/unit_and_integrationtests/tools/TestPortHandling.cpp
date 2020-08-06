#include <GlobalData.hpp>
#include <interfaces/IPlugin.hpp>
#include "base/PluginBases.hpp"
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
#include "CatchTools.hpp"
#include <initializer_list>
using namespace XPlug;

class  TestPortHandlingPlugin : public IPlugin {
protected:
	DynamicPortComponent portComponent;
public:
	TestPortHandlingPlugin() {
		//this->inf = StaticInfoComponent("No Name Plugin", "urn://LazyPlugin/ToLazyToInit", "A simple plugin wich does something... at least it exists", "LGPL", "Me", "http://myhompeage.never.existed.com")
	};
	void fillPorts() {
		this->portComponent.addPort(std::make_unique<MonoPort>("In0", PortDirection::Input));
		this->portComponent.addPort(std::make_unique<StereoPort>("In1", PortDirection::Input));
		this->portComponent.addPort(std::make_unique<Surround5_1Port>("In2", PortDirection::Input));
		this->portComponent.addPort(std::make_unique<MonoPort>("Out0", PortDirection::Output));
		this->portComponent.addPort(std::make_unique<StereoPort>("Out1", PortDirection::Output));
		this->portComponent.addPort(std::make_unique<Surround5_1Port>("Out2", PortDirection::Output));

		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiIn0", PortDirection::Input));
		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiIn1", PortDirection::Input));
		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiOut0", PortDirection::Output));
	}


	virtual void processAudio() override{}
	virtual void init() override{}
	virtual void deinit() override{}
	virtual void activate() override{}
	virtual void deactivate() override	{}
	virtual IInfoComponent* getInfoComponent() override{ return nullptr;}
	virtual IFeatureComponent* getFeatureComponent() override { return nullptr; }
	virtual IPortComponent* getPortComponent() override
	{
		return &portComponent;
	}

};

TEST_CASE("Testing Porthandling Iterationfunctions") {
	auto plug = new TestPortHandlingPlugin();
	plug->fillPorts();
	auto emptyPlug = new TestPortHandlingPlugin();
	REQUIRE_MESSAGE(plug != nullptr, "Error, cant initialize TestPortHandlingPlugin");
	REQUIRE_MESSAGE(emptyPlug != nullptr, "Error, cant initialize EmptyPortPlugin");
	size_t counter = 0;

	iteratePorts<IPort>(plug, [&counter ](IPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 9, "Couldnt iterate all Audioports");
	// Testing iteration over Audioports, not filtered by direction
	counter = 0;
	iteratePorts<IAudioPort>(plug, [&counter ](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
	});
	REQUIRE_MESSAGE(counter == 6, "Couldnt iterate all Audioports");

	// Testing iteration over AUuioports, filtered by inputports
	counter = 0;
	iteratePorts<IAudioPort>(plug,PortDirection::Input, [&counter ](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
	});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all Audioports");

	// Testing iteration over AUuioports,  filtered by outputports. Outputportsnames are in the array at index 3, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IAudioPort>(plug, PortDirection::Output, [&counter ](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
	});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all Audioports");


	// Testing iteration over MidiPorts, not filtered by direction. Midiportnames are in the array at index 6, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IMidiPort>(plug, [&counter ](IMidiPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all MidiPorts");

	// Testing iteration over MidiPorts, filtered by inputports. Midiportnames are in the array at index 6, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IMidiPort>(plug, PortDirection::Input, [&counter ](IMidiPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 2, "Couldnt iterate all MidiPorts");

	// Testing iteration over MidiPorts, filtered by outputports. MidiOutput portnames are in the array at index 8, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IMidiPort>(plug, PortDirection::Output, [&counter ](IMidiPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 1, "Couldnt iterate all MidiPorts");

	counter = 0;
	iteratePorts<IPort>(plug, [&counter](IPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 9, "Couldnt iterate all Audioports");
	// Testing iteration over Audioports, not filtered by direction
	counter = 0;
	iteratePorts<IAudioPort>(plug, [&counter](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 6, "Couldnt iterate all Audioports");

	// Testing iteration over AUuioports, filtered by inputports
	counter = 0;
	iteratePorts<IAudioPort>(plug, PortDirection::Input, [&counter](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all Audioports");

	// Testing iteration over AUuioports,  filtered by outputports. Outputportsnames are in the array at index 3, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IAudioPort>(plug, PortDirection::Output, [&counter](IAudioPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all Audioports");


	// Testing iteration over MidiPorts, not filtered by direction. Midiportnames are in the array at index 6, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IMidiPort>(plug, [&counter](IMidiPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 3, "Couldnt iterate all MidiPorts");

	// Testing iteration over MidiPorts, filtered by inputports. Midiportnames are in the array at index 6, so add this to the counter on data access.
	counter = 0;
	iteratePorts<IMidiPort>(plug, PortDirection::Input, [&counter](IMidiPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 2, "Couldnt iterate all MidiPorts");

	// Testing iteration over MidiPorts, filtered by inputports. Midiportnames are in the array at index 6, so add this to the counter on data access.
	counter = 0;
	iteratePortsFlat(plug, [&counter](IPort* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 21, "Couldnt iterate all Ports");


	iteratePorts<IPort>(emptyPlug,[](IPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IAudioPort>(emptyPlug, [](IAudioPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IAudioPort>(emptyPlug, PortDirection::Input, [](IAudioPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IAudioPort>(emptyPlug, PortDirection::Output, [](IAudioPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IMidiPort>(emptyPlug, [](IMidiPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IMidiPort>(emptyPlug, PortDirection::Input, [](IMidiPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false; });
	iteratePorts<IMidiPort>(emptyPlug, PortDirection::Output, [](IMidiPort* t, size_t ind) { ASSERT_MESSAGE("Error, empty Port is iterated");  return false;});
	delete plug;
	delete emptyPlug;
}

TEST_CASE("Testing Porthandling Indexing functions") {
	auto plug = new TestPortHandlingPlugin();
	plug->fillPorts();
	auto emptyPlug = new TestPortHandlingPlugin();
	REQUIRE_MESSAGE(plug != nullptr, "Error, cant initialize TestPortHandlingPlugin");
	REQUIRE_MESSAGE(emptyPlug != nullptr, "Error, cant initialize EmptyPortPlugin");
	SECTION("Port testing") {
		REQUIRE_MESSAGE(getPortAt<IPort>(plug, 0)->getPortName() == "In0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IPort>(plug, 8)->getPortName() == "MidiOut0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IPort>(plug, 1, PortDirection::Input)->getPortName() == "In1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IPort>(plug, 3, PortDirection::Input)->getPortName() == "MidiIn0", "Error while getting Audioport");
	} 
	SECTION("Audioport testing") {
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 0)->getPortName() == "In0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 1)->getPortName() == "In1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 2)->getPortName() == "In2", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 3)->getPortName() == "Out0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 4)->getPortName() == "Out1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 5)->getPortName() == "Out2", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 6) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(emptyPlug, 0) == nullptr, "Error, should got nullptr");

		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 0, PortDirection::Input)->getPortName() == "In0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 1, PortDirection::Input)->getPortName() == "In1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 2, PortDirection::Input)->getPortName() == "In2", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 3, PortDirection::Input) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(emptyPlug, 0, PortDirection::Input) == nullptr, "Error, should got nullptr");

		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 0, PortDirection::Output)->getPortName() == "Out0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 1, PortDirection::Output)->getPortName() == "Out1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 2, PortDirection::Output)->getPortName() == "Out2", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(plug, 3, PortDirection::Output) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IAudioPort>(emptyPlug, 0, PortDirection::Output) == nullptr, "Error, should got nullptr");
	}

	SECTION("Midiport testing") {
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 0)->getPortName() == "MidiIn0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 1)->getPortName() == "MidiIn1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 2)->getPortName() == "MidiOut0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 6) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(emptyPlug, 0) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 0, PortDirection::Input)->getPortName() == "MidiIn0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 1, PortDirection::Input)->getPortName() == "MidiIn1", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 5, PortDirection::Input) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(emptyPlug, 0, PortDirection::Input) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 0, PortDirection::Output)->getPortName() == "MidiOut0", "Error while getting Audioport");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(plug, 7, PortDirection::Output) == nullptr, "Error, should got nullptr");
		REQUIRE_MESSAGE(getPortAt<IMidiPort>(emptyPlug, 0, PortDirection::Output) == nullptr, "Error, should got nullptr");
	}
	delete plug;
	delete emptyPlug;
}

TEST_CASE("Testing Porthandling size functions") {
	auto plug = new TestPortHandlingPlugin();
	plug->fillPorts();
	auto emptyPlug = new TestPortHandlingPlugin();
	REQUIRE_MESSAGE(plug != nullptr, "Error, cant initialize TestPortHandlingPlugin");
	REQUIRE_MESSAGE(emptyPlug != nullptr, "Error, cant initialize EmptyPortPlugin");
	SECTION("Sizefunctions") {
		REQUIRE_MESSAGE(getNumberOfPorts<IPort>(plug,PortDirection::All)==9, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IPort>(plug, PortDirection::Input) == 5, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IPort>(plug, PortDirection::Output) == 4, "Invalid size.");

		REQUIRE_MESSAGE(getNumberOfPorts<IAudioPort>(plug, PortDirection::All) == 6, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IAudioPort>(plug, PortDirection::Input) == 3, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IAudioPort>(plug, PortDirection::Output) == 3, "Invalid size.");

		REQUIRE_MESSAGE(getNumberOfPorts<IMidiPort>(plug, PortDirection::All) == 3, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IMidiPort>(plug, PortDirection::Input) == 2, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IMidiPort>(plug, PortDirection::Output) == 1, "Invalid size.");

		REQUIRE_MESSAGE(getNumberOfPorts<IPort>(emptyPlug, PortDirection::All) == 0, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IAudioPort>(emptyPlug, PortDirection::All) == 0, "Invalid size.");
		REQUIRE_MESSAGE(getNumberOfPorts<IMidiPort>(emptyPlug, PortDirection::All) == 0, "Invalid size.");

	}
	delete plug;
	delete emptyPlug;
}


TEST_CASE("Testing Audiochannel functions") {
	auto plug = new TestPortHandlingPlugin();
	plug->fillPorts();
	auto emptyPlug = new TestPortHandlingPlugin();
	REQUIRE_MESSAGE(plug != nullptr, "Error, cant initialize TestPortHandlingPlugin");
	REQUIRE_MESSAGE(emptyPlug != nullptr, "Error, cant initialize EmptyPortPlugin");

	REQUIRE_MESSAGE(getAudioChannelCount(plug)==18,"Error, invalid size." );
	REQUIRE_MESSAGE(getAudioChannelCount(plug,PortDirection::Input) == 9, "Error, invalid size.");
	REQUIRE_MESSAGE(getAudioChannelCount(plug, PortDirection::Output) == 9, "Error, invalid size.");
	REQUIRE_MESSAGE(getAudioChannelCount(plug) == 18, "Error, invalid size.");
	REQUIRE_MESSAGE(getAudioChannelCount(emptyPlug) == 0, "Error, invalid size.");
	for (int i = 0; i < 18; i++) {
		REQUIRE_MESSAGE(getAudioChannelFromIndex(plug, i) != nullptr, "Error, there should be an channel and not an nullptr.");
		REQUIRE_MESSAGE(getAudioChannelFromIndex(emptyPlug, i) == nullptr, "Error, the empty plugin should not contian channels.");
	}
	size_t counter = 0;

	iterateAudioChannels(plug, [&counter](IAudioPort*p,IAudioChannel* t, size_t ind) {
		REQUIRE_MESSAGE(counter == ind, "Error, index is not correct");
		counter++;
		return false;
		});
	REQUIRE_MESSAGE(counter == 18, "Couldnt iterate all Audiochannels");

	delete plug;
	delete emptyPlug;
}