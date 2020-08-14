#include "IFormatTestSuite.hpp"
#include "ladspa.h"
#include "tools/LibLoading.hpp"
#include <vector>
using namespace XPlug;
/**
 * @brief LADSPA Test Suite for XValidate/system_tests
 */
class LADSPATestSuite : public FormatTestSuiteBase
{
public:
  virtual std::string getFormatName() override { return "LADSPA"; }

  virtual bool isSupported(std::string pluginPath) override
  {
    auto lib = LoadLib(pluginPath.c_str());
    if (lib == nullptr)
      return false;
    bool isSupp =
      LoadFunc<LADSPA_Descriptor_Function>(lib, "ladspa_descriptor") != nullptr;
    UnloadLib(lib);
    return isSupp;
  }

  virtual SucceedState run() override
  {
    size_t sampleRate = 512;
    SucceedState ret = TEST_SUCCEEDED;
    auto lib = LoadLib(data.pluginPath.c_str());
    if (lib == nullptr)
      return false;
    auto ladspa_descriptor_fnc =
      LoadFunc<LADSPA_Descriptor_Function>(lib, "ladspa_descriptor");
    size_t index = 0;
    const LADSPA_Descriptor* desc = nullptr;
    // All things loaded. Than Run normal execution of Plugin.
    while (desc = ladspa_descriptor_fnc(index++), desc != nullptr) {
      auto handle = desc->instantiate(desc, sampleRate);
      desc->activate(handle);
      auto portData = std::vector<LADSPA_Data*>(desc->PortCount);
      for (unsigned long i = 0; i < desc->PortCount; i++) {
        portData[i] = new LADSPA_Data[sampleRate];
        desc->connect_port(handle, i, portData[i]);
      }
      desc->run(handle, sampleRate);
      for (unsigned long i = 0; i < desc->PortCount; i++) {
        delete[] portData[i];
      }
      desc->deactivate(handle);
      desc->cleanup(handle);
    }

    return ret;
  }
};
REGISTER_TEST_SUITE(LADSPATestSuite);