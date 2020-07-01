#include <base/LazyPlugin.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace XPlug;
class MidiForwarder :public LazyPlugin {
public:
	MidiForwarder() {
		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiIn", PortType::MIDI, PortDirection::Input));
		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiOut", PortType::MIDI, PortDirection::Output));
		this->portComponent.addPort(std::make_unique<QueueMidiPort>("MidiOut2", PortType::MIDI, PortDirection::Output));
		this->featureComp.detectFeatures(this->getPortComponent());
		this->inf.name = "MidiForwarder";
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