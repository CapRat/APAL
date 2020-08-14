#include "IFormatTestSuite.hpp"
#include "VST3Module.hpp"
#include "tools/LibLoading.hpp"
#include <functional>
#include <vector>
#include <vst/ivstaudioprocessor.h>
#include <vst/ivsteditcontroller.h>
#include <vst/ivstevents.h>
#include <vst/ivstprocesscontext.h>
using namespace XPlug;
using namespace Steinberg;
using namespace Vst;
/**
* @brief Simple macro, which can be used in VST3TEstSuite to report errors, if a fail is happening.
*/
#define TESTFAIL_IF(cond)                                                      \
  if (cond) {                                                                  \
    GlobalLog().logN(module.getLastError(), LoggerValue::ERROR);               \
    return TEST_FAILED;                                                        \
  }

/**
 * @brief  Simple Testclass, derived from FormatTestSuiteBase to Test VST3
 * Format. Using the VST3Module to make VST3 calls.
 */
class VST3TestSuite : public FormatTestSuiteBase
{
protected:
public:
  virtual std::string getFormatName() override { return "VST3"; }

  virtual SucceedState run() override
  {
    SucceedState res = TEST_SUCCEEDED;
    GlobalLog().logN("Try to load functions from VST3 Module.",
                     LoggerValue::INFO);
    VST3Module module(data.pluginPath);
    TESTFAIL_IF(module.getLastError() != "");

    GlobalLog().logN("Try to get Components from VST3 Module.",
                     LoggerValue::INFO);
    TESTFAIL_IF(module.initialize())
    else
    {
      GlobalLog().logN("Successfully loaded Components from Module.",
                       LoggerValue::INFO);
    }
    GlobalLog().logN("Try allocating Memory.", LoggerValue::INFO);
    TESTFAIL_IF(module.allocateData(512));
    GlobalLog().logN("Try processing.", LoggerValue::INFO);
    TESTFAIL_IF(module.run())
    else
    {
      GlobalLog().logN("Successfully processed  Plugin", LoggerValue::INFO);
    }
    TESTFAIL_IF(module.deinitalize())
    else
    {
      GlobalLog().logN("Successfully cleaned up  Plugin", LoggerValue::INFO);
    }
    return res;
  }

  virtual bool isSupported(std::string pluginPath) override
  {
    auto lib = LoadLib(pluginPath.c_str());
    if (lib == nullptr)
      return false;
    bool isSupp = LoadFunc<GetFactoryProc>(lib, "GetPluginFactory") != nullptr;
    UnloadLib(lib);
    return isSupp;
  }
};
REGISTER_TEST_SUITE(VST3TestSuite);
