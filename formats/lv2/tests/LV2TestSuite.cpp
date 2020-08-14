#include "IFormatTestSuite.hpp"
#include "LV2Module.hpp"
#include "lilv.h"
#include "tools/LibLoading.hpp"
#include <algorithm>
#include <array>
#include <lv2/atom/forge.h>
#include <lv2/core/lv2.h>
#include <lv2/midi/midi.h>
#include <lv2/urid/urid.h>
#include <map>
#include <vector>
using namespace XPlug;

/**
 * @brief Simple Testclass, derived from FormatTestSuiteBase to Test LV2 Format.
 * Using the LV2Module to make LV2 calls.
 */
class LV2TestSuite : public FormatTestSuiteBase
{

public:
  LV2Module* module = nullptr;

  LV2TestSuite() {}
  ~LV2TestSuite()
  {
    if (this->module != nullptr)
      delete this->module;
  }
  inline virtual void initialize(TestSuiteData _data) override
  {
    this->data = _data;
    this->module = new LV2Module(this->data.pluginPath);
  }

  virtual std::string getFormatName() override { return "LV2"; }

  virtual SucceedState run() override
  {
    double sampleRate = 512;
    // size_t sampleCount = 512;
    for (auto plug : this->module->plugins) {
      plug.verify();
      plug.instantiate(sampleRate);
      plug.allocateAndConnectPorts(512);
      plug.activate();
      for (auto& p : plug.ports) {
        if (p.type == PortType::Midi && p.dir == Direction::Input) {
          p.addMidiMsg(0x1, 0x2, 0x3);
          p.addMidiMsg(0xFF, 0xFF, 0xFF);
        }
      }
      plug.run(sampleRate);
      plug.deactivate();
    }
    return TEST_SUCCEEDED;
  }

  virtual bool isSupported(std::string pluginPath) override
  {
    auto lib = LoadLib(pluginPath.c_str());
    if (lib == nullptr)
      return false;
    bool isSupp =
      LoadFunc<LV2_Descriptor_Function>(lib, "lv2_descriptor") != nullptr;
    UnloadLib(lib);
    return isSupp;
  }
};
REGISTER_TEST_SUITE(LV2TestSuite);