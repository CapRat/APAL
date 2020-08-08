#include "IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include <functional>
//#include ""
//#include <pluginterfaces/base/ipluginbase.h>
#include "VST3Module.hpp"
#include <vector>
#include <vst/ivstaudioprocessor.h>
#include <vst/ivsteditcontroller.h>
#include <vst/ivstevents.h>
#include <vst/ivstprocesscontext.h>
using namespace XPlug;
using namespace Steinberg;
using namespace Vst;

class VST3TestSuite : public FormatTestSuiteBase
{
protected:
  XPlug::library_t pluginLibrary;  // The pluginlibrary
  GetFactoryProc plugLoadFunction; // FactoryLoad Function
  void* initfnc;                   // functionpointer to init function
  void* exitfnc;                   // functionpointer to init function
  std::function<bool()>
    runInitFncLampda; // Function to call init function crossplattform. Returns
                      // Value from InitDll/InitModule/bundleEntry
  std::function<bool()>
    runExitFncLampda; // Function to call exit function crossplattform.Returns
                      // Value from  ExitDll/ExitModule/bundleExit

  Steinberg::Vst::IComponent* processorComponent = nullptr;
  Steinberg::Vst::IEditController* editController = nullptr;
  Steinberg::Vst::IAudioProcessor* audioProcessor = nullptr;
  Steinberg::IPluginFactory* factory = nullptr;
  Steinberg::IPluginFactory2* factory2 = nullptr;
  Steinberg::IPluginFactory3* factory3 = nullptr;

public:
  virtual std::string getFormatName() override { return "VST3"; }

  /*virtual SucceedState loadFunctionsFromModule() {
      SucceedState res = TEST_SUCCEEDED;
      GetError();
      this->pluginLibrary = LoadLib(data.pluginPath.c_str());
      res=test(0,this->pluginLibrary != nullptr, "Error, while loading VST3
binary, with Message :" + GetErrorStr()); if (res == TEST_FAILED)return
TEST_FAILED;
       //STATUS_RESULT("successfully loaded!");


      this->plugLoadFunction = LoadFunc<GetFactoryProc >(this->pluginLibrary,
"GetPluginFactory"); res=test(0,this->plugLoadFunction != nullptr, "Error, while
loading VST3 function, with Message :" + GetErrorStr()); if (res ==
TEST_FAILED)return TEST_FAILED; #if defined(WIN32) this->initfnc =
LoadFuncRaw(this->pluginLibrary, "InitDll"); this->exitfnc =
LoadFuncRaw(this->pluginLibrary, "ExitDll"); this->runInitFncLampda =
[this]()->bool {return ((bool (*)())this->initfnc)(); }; this->runExitFncLampda
= [this]()->bool {return ((bool (*)())this->exitfnc)(); }; #elif
defined(__APPLE__) this->initfnc = LoadFuncRaw(this->pluginLibrary,
"bundleEntry"); this->exitfnc = LoadFuncRaw(this->pluginLibrary, "bundleExit");
      this->runInitFncLampda = [this]()->bool {return ((bool
(*)(CFBundleRef))this->initfnc)(); }; this->runExitFncLampda = [this]()->bool
{return ((bool (*)())this->exitfnc)(); }; #elif defined(__linux__) this->initfnc
= LoadFuncRaw(this->pluginLibrary, "ModuleEntry"); this->exitfnc =
LoadFuncRaw(this->pluginLibrary, "ModuleExit"); this->runInitFncLampda =
[this]()->bool {return ((bool (*)(void*))this->initfnc)((void*)pluginLibrary);
}; this->runExitFncLampda = [this]()->bool {return ((bool
(*)())this->exitfnc)(); }; #else assert("Warning, cant determinate Sytem. No
Initfunction(like InitDll) is executed nor testeted"); #endif //!WIN32

      res=test(7,!(this->initfnc == nullptr || this->exitfnc == nullptr), "No
initialisation functions given, for VST3 Module." + GetErrorStr()); return res;
  }*/

  /*virtual SucceedState loadComponentsFromModule() {
      SucceedState res = TEST_SUCCEEDED;
      this->factory = plugLoadFunction();
      if(this->factory != nullptr)
      res= test(1,(this->factory != nullptr), "Error while loading factory.
  Factory is NULL"); res &= test(1,(this->factory->countClasses() >= 2), "Error
  There are less then 2 classes in Factory. There must be at least one
  IAudioProcessor and one IEditController class."); PFactoryInfo info; res &=
  test(2,(this->factory->getFactoryInfo(&info) == kResultOk), "Cant get
  FactoryInfo from factory");
      this->factory->queryInterface(IPluginFactory3::iid, (void**)&factory3);
      this->factory->queryInterface(IPluginFactory2::iid, (void**)&factory2);
      for (int i = 0; i < this->factory->countClasses(); i++) {
          // Get Class Info
          PClassInfoW ci3;
          if (factory3 != nullptr) {
              this->factory3->getClassInfoUnicode(i, &ci3);
          }
          else if (factory2 != nullptr) {
              Steinberg::PClassInfo2 ci2;
              this->factory2->getClassInfo2(i, &ci2);
              ci3.fromAscii(ci2);
          }
          else {
              Steinberg::PClassInfo ci;
              this->factory->getClassInfo(i, &ci);
              PFactoryInfo facInfo;
              res &= test(1, (this->factory->getFactoryInfo(&facInfo) ==
  kResultOk), "Cant get FactoryInfo from factory"); Steinberg::PClassInfo2
  ci2(ci.cid, ci.cardinality, ci.category, ci.name, 0, NULL, facInfo.vendor,
  "0.0.0.0", "VST 3.0.0"); ci3.fromAscii(ci2);
          }

          if (strcmp(ci3.category, kVstAudioEffectClass) == 0) {//Class is an
  Audio Effect Processor res &= test(2,( (this->factory->createInstance(ci3.cid,
  Vst::IComponent::iid, (void**)&processorComponent)
                  == kResultOk)), "Error, could not create IComponentobject
  successfully"); std::string x; int i = 0; while (ci3.name[i] != NULL) {
                  x.push_back(static_cast<char>(ci3.name[i++]));
              }
              res &= test(2, (processorComponent->initialize(nullptr) ==
  kResultOk), std::string("Error while initialising Component ") + x); TUID
  controllerCID; res &= test(2,
  processorComponent->getControllerClassId(controllerCID) == kResultTrue,
  "Error,while getting ControllerID from IComponent."); auto controllerFUID =
  FUID::fromTUID(controllerCID); res &= test(2, controllerFUID.isValid(),
  "Error, ControllerID from IComponent is not valid."); res &= test(2,
  (this->factory->createInstance(controllerCID, Vst::IEditController::iid,
  (void**)&editController)
                  == kResultOk), "Error while creating EditController from
  Factory."); res &= test(2, editController->initialize(nullptr) == kResultOk,
  "Error while initializing EditController"); res &= test(2,
  (processorComponent->queryInterface(Vst::IAudioProcessor::iid,
  (void**)&audioProcessor) == kResultOk), "Error, while retreiving the
  AudioInterface");
          }
          else {
              continue;
          }
      }
      return res;
  }*/

  /* virtual SucceedState processTestProcessing() {
       std::vector<SpeakerArrangement>
   inputs(this->processorComponent->getBusCount(MediaTypes::kAudio,
   BusDirections::kInput)); std::vector<SpeakerArrangement>
   outputs(this->processorComponent->getBusCount(MediaTypes::kAudio,
   BusDirections::kOutput)); SymbolicSampleSizes size =
   this->audioProcessor->canProcessSampleSize(kSample64) ? kSample64 :
   kSample32; size_t i = 0; SucceedState res= TEST_SUCCEEDED; for(auto
   &sArr:inputs ) res &= test(4,
   this->audioProcessor->getBusArrangement(BusDirections::kInput, i++, sArr) ==
   kResultOk, "Error, Couldnt get Busarrangement"); i = 0; for (auto& sArr :
   outputs) res &= test(4,
   this->audioProcessor->getBusArrangement(BusDirections::kOutput, i++, sArr) ==
   kResultOk, "Error, Couldnt get Busarrangement"); ProcessDataClass
   pData(inputs, outputs,SymbolicSampleSizes::kSample32,512);
       this->audioProcessor->process(pData);
       return res;
   }*/
#define TESTFAIL_IF(cond)                                                      \
  if (cond) {                                                                  \
    GlobalLog().logN(module.getLastError(), LoggerValue::ERROR);               \
    return TEST_FAILED;                                                        \
  }
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
