#include <base/PluginBases.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace XPlug;
class MidiForwarder :public LazyPlugin {
public:
	MidiForwarder() {
		this->portComponent->addPort(std::make_shared<QueueMidiPort>("MidiIn", PortDirection::Input));
		this->portComponent->addPort(std::make_shared<QueueMidiPort>("MidiOut", PortDirection::Output));
		this->portComponent->addPort(std::make_shared<QueueMidiPort>("MidiOut2", PortDirection::Output));
		this->featureComponent->detectFeatures(this->getPortComponent());
		this->infoComponent->pluginName = "MidiForwarder";
        this->infoComponent->creatorName="Benjamin Heisch";
        this->infoComponent->pluginUri = "http://xplug_plug.in/examples/MidiForwarder";
	}
	// Geerbt über IPlugin
	virtual void processAudio() override {
		auto in0 = getPortAt<IMidiPort>(this, 0,PortDirection::Input);
		auto out0 = getPortAt<IMidiPort>(this, 0, PortDirection::Output);
		auto out1 = getPortAt<IMidiPort>(this, 1, PortDirection::Output);
		while (!in0->empty()) {
			auto midiMsg = in0->get();
			auto midiMsg2= midiMsg;
			midiMsg[1]=midiMsg[1]*2;
			out0->feed(std::move(midiMsg));
			out1->feed(std::move(midiMsg2));
		}
	}
	virtual void init() override {

	}
	virtual void deinit() override {

	}
	virtual void activate() override {

	}
	virtual void deactivate() override {

	}
	//virtual PluginInfo* getPluginInfo() override;

};
REGISTER_PLUGIN(MidiForwarder);
