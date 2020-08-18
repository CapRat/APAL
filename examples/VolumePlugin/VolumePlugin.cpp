#include <base/PluginBases.hpp>
#include <base/Ports.hpp>
#include <tools/PortHandling.hpp>
using namespace APAL;
/**
 * @brief VolumePlguin class, which forwards audiodata and just changes the
 * Volume. For an easy start its derived from LazyPlugin, to start easily.
 */
class VolumePlugin : public LazyPlugin
{
protected:
  // Define an Input and an Outputport. (To optimize accesstime, not like in
  // MidiForwarder)
  std::shared_ptr<IAudioPort> in0 =
    std::make_shared<MonoPort>("In0", PortDirection::Input);
  std::shared_ptr<IAudioPort> out0 =
    std::make_shared<MonoPort>("Out0", PortDirection::Output);

public:
  /**
   * @brief Constructor, which adding the local defined Ports, to the
   * PortComponent. Its also changed the needed Plugininformation.
   */
  VolumePlugin()
  {
    // Adding  the local ports to the portcomponent, so the plugininfrastructure
    // can process it.
    this->portComponent->addPort(in0);
    this->portComponent->addPort(out0);
    // Changing the Information about the Plugin, because the default Values
    // are... not enough ;)
    this->infoComponent->pluginName = "VolumePlugin";
    this->infoComponent->creatorName = "Benjamin Heisch";
    this->infoComponent->pluginUri =
      "http://APAL_plug.in/examples/VolumePlugin";
  }

  // Geerbt über IPlugin
  virtual void processAudio() override
  {
    // Simple loop over the Dat and process.
    // Currently no Volumechanging is used, but it will be added later, when the
    // Framework supports Parameter, so the Volume can be adjustet.
    for (size_t i = 0; i < in0->size(); i++) {
      for (size_t s = 0; s < in0->getSampleCount(); s++) {
        if (in0->at(i)->getData() != nullptr &&
            out0->at(i)->getData() != nullptr)
          out0->at(i)->getData()[s] = in0->at(i)->getData()[s];
      }
    }
  }
  virtual void init() override {}
  virtual void deinit() override {}
  virtual void activate() override {}
  virtual void deactivate() override {}
};
// Its very important to Register a Plugin in the GlobalData object, which is
// accessed through the GlobalData() Function.
// The easiest way ist to just use this REGISTER_PLUGIN macro. But use it in
// your sourcefile, because a static variable will be declared. And that just
// works in Sourcefiles.
REGISTER_PLUGIN(VolumePlugin);
