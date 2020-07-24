#include <base/PluginBases.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace XPlug;
class VolumePlugin :public LazyPlugin {
protected:
    std::shared_ptr<IAudioPort> in0= std::make_shared<MonoPort>("In0", PortDirection::Input);
    std::shared_ptr<IAudioPort> out0= std::make_shared<MonoPort>("Out0", PortDirection::Output);
public:
    VolumePlugin()
    {
        this->portComponent->addPort(in0);
        this->portComponent->addPort(out0);
        this->infoComponent->pluginName = "VolumePlugin";
    }

    // Geerbt �ber IPlugin
    virtual void processAudio() override
    {
      //  auto in0 = getPortAt<IAudioPort>(this, 0,PortDirection::Input);
       // auto out0 = getPortAt<IAudioPort>(this, 0, PortDirection::Output);
        for (int i = 0; i < in0->size(); i++) {
            for (int s = 0; s < in0->getSampleSize(); s++) {
                out0->at(i)->getData()[s] = in0->at(i)->getData()[s] * 0.5f;
                /* if (outputs[0].channels[i].data64 != nullptr) {
                     outputs[0].channels[i].data64[s] = inputs[0].channels[i].data64[s] * 0.5;
                 }*/
            }
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
REGISTER_PLUGIN(VolumePlugin);
