#include <base/LazyPlugin.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace XPlug;
class VolumePlugin :public LazyPlugin {
public:
    VolumePlugin()
    {
        this->portComponent.addPort(std::make_unique<MonoPort>("In0", PortType::Audio, PortDirection::Input));
        this->portComponent.addPort(std::make_unique<MonoPort>("Out0", PortType::Audio, PortDirection::Output));
        this->inf.name = "VolumePlugin";
    }

    // Geerbt über IPlugin
    virtual void processAudio() override
    {
        auto in0 = getAudioInputPortAt(this, 0);
        auto out0 = getAudioOutputPortAt(this, 0);
        for (int i = 0; i < in0->size(); i++) {
            for (int s = 0; s < in0->getSampleSize(); s++) {
                out0->at(i)->getData32()[s] = in0->at(i)->getData32()[s] * 0.5f;
                /* if (outputs[0].channels[i].data64 != nullptr) {
                     outputs[0].channels[i].data64[s] = inputs[0].channels[i].data64[s] * 0.5;
                 }*/
            }
        }

        /*   for (int i = 0; i < inputs[0].channels.size(); i++) {
               outputs[0].channels[i].data32 = inputs[0].channels[i].data32;
               outputs[0].channels[i].data64 = inputs[0].channels[i].data64;
           }*/

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
REGISTER_PLUGIN(VolumePlugin);
