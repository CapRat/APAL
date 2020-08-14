#include "IFormatTestSuite.hpp"
#include "VST2Module.hpp"
using namespace XPlug;
typedef AEffect* (*VSTPluginMain)(audioMasterCallback);

/**
 * @brief Simple Testclass, derived from FormatTestSuiteBase to Test VST2 Format.
 * Using the VST2Module to make VST2 calls.
 */
class VST2TestSuite : public FormatTestSuiteBase
{
public:
  virtual std::string getFormatName() override { return "VST2"; }

  virtual SucceedState run() override
  {
    // size_t sampleSize = 512;
    VST2Module module(data.pluginPath);
    module.intialise();
    module.allocate(512);
    module.run();
    char msg[3] = { 0x1, 0x2, 0x3 };
    module.sendMidi(msg);
    module.deinitialize();
    module.free();

    // MIDI TEST

    // effect->dispatcher(effect, effCanDo, 0, 0, (void*)"sendVstMidiEvent", 0);

    return TEST_SUCCEEDED;
  }

  virtual bool isSupported(std::string pluginPath) override
  {
    auto lib = LoadLib(pluginPath.c_str());
    if (lib == nullptr)
      return false;
    bool isSupp = LoadFunc<VSTPluginMain>(lib, "VSTPluginMain") != nullptr;
    UnloadLib(lib);
    return isSupp;
  }
};
REGISTER_TEST_SUITE(VST2TestSuite);