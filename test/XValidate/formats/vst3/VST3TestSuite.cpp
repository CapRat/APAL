#include "../../IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include <functional>
//#include ""
//#include <pluginterfaces/base/ipluginbase.h>
#include <vst/ivstaudioprocessor.h>
#include <vst/ivsteditcontroller.h>
#include <vst/ivstprocesscontext.h>
#include <vst/ivstevents.h>
#include<vector>

using namespace XPlug;
using namespace Steinberg;
using namespace Vst;

/**
 * @brief Implementation of the IEventList interfacem but without query ,addref and release functions, because it should not be needed.
*/
class EventList : public IEventList {
private:
    std::vector<Event> events;
public:
    virtual int32 getEventCount()override { return events.size(); }
    virtual ~EventList() {};
    virtual tresult getEvent(int32 index, Event& e) override {
        if (index < events.size()) {
            e = events[index];
            return kResultOk;
        }
        return kResultFalse;
    }
    virtual tresult addEvent(Event& e) override {
        this->events.push_back(e);
        return kResultOk;
    }

    //dont implement that stupid COM Interface, cause it should not be used here...
    virtual tresult queryInterface(const TUID _iid, void** obj) override { return kNoInterface; }
    virtual uint32  addRef() override { return 0; }
    virtual uint32  release() override { return 0; }
};



/**
 * @brief Simple class to handle allocating and freeing of Memory for ProcessData struct.
 */
struct ProcessDataClass :public ProcessData {
protected:
    std::vector<AudioBusBuffers> inputAudioBusBuffers;
    std::vector<AudioBusBuffers> outputAudioBusBuffers;
    EventList inputEventsClass;
    EventList outputEventsClass;
    // Just gets the numbers of channels in an Arrangement. (Hemmingweight)
    static inline size_t getNumberOfChannel(SpeakerArrangement arr)
    {
        size_t number = 0;
        while (arr)
        {
            if (arr & (SpeakerArrangement)1) ++number;
            arr >>= 1;
        }
        return number;
    }
public:
    /**
     * @brief Allocates everything in the Object.
     * @param inputChannels number and Arrangements of inputchannels. The size indicates, how much inputs are used.
     * @param outputChannels number and Arrangements of outputchannels. The size indicates, how much outputs are used. 
     * @param sampleSize size of on Sample. Values can be 32 or 64. Values from SymbolicSampleSizes
     * @return 
    */
    ProcessDataClass(std::vector<SpeakerArrangement> inputChannels, std::vector<SpeakerArrangement> outputChannels, SymbolicSampleSizes sampleSize) {
        this->processMode = ProcessModes::kRealtime;
        this->symbolicSampleSize = sampleSize;
        this->processContext = nullptr;
        this->numSamples = 512;
        this->numInputs = inputChannels.size();
        this->numOutputs = outputChannels.size();
        inputAudioBusBuffers = std::vector<AudioBusBuffers>(this->numInputs);
        outputAudioBusBuffers=std::vector<AudioBusBuffers>(this->numOutputs);
        this->inputs = inputAudioBusBuffers.data();
        this->outputs = outputAudioBusBuffers.data();
        this->inputEvents = &inputEventsClass;
        this->outputEvents = &outputEventsClass;
        this->inputParameterChanges = nullptr;
        this->outputParameterChanges = nullptr;
        auto allocator = [this](std::vector<AudioBusBuffers>& buffers, std::vector<SpeakerArrangement>& channelConfig) {
            for (int i = 0; i < buffers.size(); i++) {
                buffers[i].numChannels = getNumberOfChannel(channelConfig[i]);
                buffers[i].silenceFlags = NULL;
                if (this->symbolicSampleSize == SymbolicSampleSizes::kSample32) {
                    buffers[i].channelBuffers32 = new Sample32 * [this->inputs[i].numChannels];
                    for (int j = 0; j < buffers[i].numChannels; j++) {
                        buffers[i].channelBuffers32[j] = new Sample32[this->numSamples];
                        std::fill_n(buffers[i].channelBuffers32[j], this->numSamples, 0);
                    }
                }
                else {
                    buffers[i].channelBuffers64 = new Sample64 * [this->inputs[i].numChannels];
                    for (int j = 0; j < buffers[i].numChannels; j++) {
                        buffers[i].channelBuffers64[j] = new Sample64[this->numSamples];
                        std::fill_n(buffers[i].channelBuffers64[j], this->numSamples, 0);
                    }
                }
            }
        };
        allocator(inputAudioBusBuffers, inputChannels);
        allocator(outputAudioBusBuffers, outputChannels);
    }
    /**
     * @brief Deletes any allocated Memory.
     */
    ~ProcessDataClass() {
        auto deleter = [this](std::vector<AudioBusBuffers> buffers) {
            for (auto buffer : buffers) {
                if (this->symbolicSampleSize == SymbolicSampleSizes::kSample32) {
                    for (int j = 0; j < buffer.numChannels; j++) {
                        delete[] buffer.channelBuffers32[j];
                    }
                    delete[] buffer.channelBuffers32;
                }
                else {
                    for (int j = 0; j < buffer.numChannels; j++) {
                        delete[] buffer.channelBuffers64[j];
                    }
                    delete[] buffer.channelBuffers64;
                }
            }
        };
        deleter(inputAudioBusBuffers);
        deleter(outputAudioBusBuffers);
    }
};


class VST3TestSuite :public FormatTestSuiteBase {
protected:
    XPlug::library_t pluginLibrary; // The pluginlibrary
    GetFactoryProc plugLoadFunction; // FactoryLoad Function
    void* initfnc; // functionpointer to init function
    void* exitfnc; // functionpointer to init function
    std::function<bool()>runInitFncLampda; // Function to call init function crossplattform. Returns Value from InitDll/InitModule/bundleEntry
    std::function<bool()>runExitFncLampda; // Function to call exit function crossplattform.Returns Value from  ExitDll/ExitModule/bundleExit

    Steinberg::Vst::IComponent* processorComponent = nullptr;
    Steinberg::Vst::IEditController* editController = nullptr;
    Steinberg::Vst::IAudioProcessor* audioProcessor = nullptr;
    Steinberg::IPluginFactory* factory = nullptr;
    Steinberg::IPluginFactory2* factory2 = nullptr;
    Steinberg::IPluginFactory3* factory3 = nullptr;
public:

    virtual std::string getFormatName() override { return "VST3"; }

    virtual SucceedState runPerformance() override
    {
        return SucceedState();
    }
    virtual SucceedState loadFunctionsFromModule() {
        SucceedState res = TEST_SUCCEEDED;
        GetError();
        this->pluginLibrary = LoadLib(data.pluginPath.c_str());
        res=test(0,this->pluginLibrary != nullptr, "Error, while loading VST3 binary, with Message :" + GetErrorStr());
        if (res == TEST_FAILED)return TEST_FAILED;
         //STATUS_RESULT("successfully loaded!");
    

        this->plugLoadFunction = LoadFunc<GetFactoryProc >(this->pluginLibrary, "GetPluginFactory");
        res=test(0,this->plugLoadFunction != nullptr, "Error, while loading VST3 function, with Message :" + GetErrorStr());
        if (res == TEST_FAILED)return TEST_FAILED;
#if defined(WIN32)
        this->initfnc = LoadFuncRaw(this->pluginLibrary, "InitDll");
        this->exitfnc = LoadFuncRaw(this->pluginLibrary, "ExitDll");
        this->runInitFncLampda = [this]()->bool {return ((bool (*)())this->initfnc)(); };
        this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#elif defined(__APPLE__)
        this->initfnc = LoadFuncRaw(this->pluginLibrary, "bundleEntry");
        this->exitfnc = LoadFuncRaw(this->pluginLibrary, "bundleExit");
        this->runInitFncLampda = [this]()->bool {return ((bool (*)(CFBundleRef))this->initfnc)(); };
        this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#elif defined(__linux__)
        this->initfnc = LoadFuncRaw(this->pluginLibrary, "ModuleEntry");
        this->exitfnc = LoadFuncRaw(this->pluginLibrary, "ModuleExit");
        this->runInitFncLampda = [this]()->bool {return ((bool (*)(void*))this->initfnc)((void*)pluginLibrary); };
        this->runExitFncLampda = [this]()->bool {return ((bool (*)())this->exitfnc)(); };
#else 
        assert("Warning, cant determinate Sytem. No Initfunction(like InitDll) is executed nor testeted");
#endif //!WIN32

        res=test(7,!(this->initfnc == nullptr || this->exitfnc == nullptr), "No initialisation functions given, for VST3 Module." + GetErrorStr());
        return res;
    }
  
    virtual SucceedState loadComponentsFromModule() {
        SucceedState res = TEST_SUCCEEDED;
        this->factory = plugLoadFunction();
        if(this->factory != nullptr)
        res= test(1,(this->factory != nullptr), "Error while loading factory. Factory is NULL");
        res &= test(1,(this->factory->countClasses() >= 2), "Error There are less then 2 classes in Factory. There must be at least one IAudioProcessor and one IEditController class.");
        PFactoryInfo info;
        res &= test(2,(this->factory->getFactoryInfo(&info) == kResultOk), "Cant get FactoryInfo from factory");
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
                res &= test(1, (this->factory->getFactoryInfo(&facInfo) == kResultOk), "Cant get FactoryInfo from factory");
                Steinberg::PClassInfo2 ci2(ci.cid, ci.cardinality, ci.category, ci.name, 0, NULL, facInfo.vendor, "0.0.0.0", "VST 3.0.0");
                ci3.fromAscii(ci2);
            }

            if (strcmp(ci3.category, kVstAudioEffectClass) == 0) {//Class is an Audio Effect Processor
                res &= test(2,( (this->factory->createInstance(ci3.cid, Vst::IComponent::iid, (void**)&processorComponent)
                    == kResultOk)), "Error, could not create IComponentobject successfully");
                std::string x;
                int i = 0;
                while (ci3.name[i] != NULL) {
                    x.push_back(static_cast<char>(ci3.name[i++]));
                }
                res &= test(2, (processorComponent->initialize(nullptr) == kResultOk), std::string("Error while initialising Component ") + x);
                TUID controllerCID;
                res &= test(2, processorComponent->getControllerClassId(controllerCID) == kResultTrue, "Error,while getting ControllerID from IComponent.");
                auto controllerFUID = FUID::fromTUID(controllerCID);
                res &= test(2, controllerFUID.isValid(), "Error, ControllerID from IComponent is not valid.");
                res &= test(2, (this->factory->createInstance(controllerCID, Vst::IEditController::iid, (void**)&editController)
                    == kResultOk), "Error while creating EditController from Factory.");
                res &= test(2, editController->initialize(nullptr) == kResultOk, "Error while initializing EditController");
                res &= test(2, (processorComponent->queryInterface(Vst::IAudioProcessor::iid, (void**)&audioProcessor) == kResultOk), "Error, while retreiving the AudioInterface");
            }
            else {
                continue;
            }
        }
        return res;
    }

    virtual SucceedState processTestProcessing() {
        std::vector<SpeakerArrangement> inputs(this->processorComponent->getBusCount(MediaTypes::kAudio, BusDirections::kInput));
        std::vector<SpeakerArrangement> outputs(this->processorComponent->getBusCount(MediaTypes::kAudio, BusDirections::kOutput));
        SymbolicSampleSizes size = this->audioProcessor->canProcessSampleSize(kSample64) ? kSample64 : kSample32;
        size_t i = 0;
        SucceedState res= TEST_SUCCEEDED;
        for(auto &sArr:inputs )
            res &= test(4, this->audioProcessor->getBusArrangement(BusDirections::kInput, i++, sArr) == kResultOk, "Error, Couldnt get Busarrangement");
        i = 0;
        for (auto& sArr : outputs)
            res &= test(4, this->audioProcessor->getBusArrangement(BusDirections::kOutput, i++, sArr) == kResultOk, "Error, Couldnt get Busarrangement");
        ProcessDataClass pData(inputs, outputs,SymbolicSampleSizes::kSample32);
        this->audioProcessor->process(pData);
        return res;
    }

    virtual SucceedState run() override
    {
       SucceedState res = TEST_SUCCEEDED;
       GlobalLog().logN("Try to load functions from VST3 Module.", LoggerValue::INFO);
       res &= loadFunctionsFromModule();
       if (res == TEST_SUCCEEDED) {
           GlobalLog().logN("Successfully loaded functions from Module.", LoggerValue::INFO);
       }
       else {
           GlobalLog().logN("Could not load functions from VST3 Module. Make sure they have correct extern \"C\" linkage and are present.", LoggerValue::STATUS);
           return res; // return now, cause more tests dont make sense.
       }
       GlobalLog().logN("Try to get Components from VST3 Module.", LoggerValue::INFO);
       res &= loadComponentsFromModule();
       if (res == TEST_SUCCEEDED) {
           GlobalLog().logN("Successfully loaded Components from Module.", LoggerValue::INFO);
       }
       else {
           GlobalLog().logN("Could not load Components from Module, make sure that they are there", LoggerValue::STATUS);
           return res; // return now, cause more tests dont make sense.
       }

       GlobalLog().logN("Try processing.", LoggerValue::INFO);
       res &= processTestProcessing();
       if (res == TEST_SUCCEEDED) {
           GlobalLog().logN("Successfully processed  Plugin.", LoggerValue::INFO);
       }
       else {
           GlobalLog().logN("Naaah, Processing wont work... Aggain!", LoggerValue::STATUS);
           return res; // return now, cause more tests dont make sense.
       }

       if (editController != nullptr)
           test(1,editController->terminate() == kResultOk, "Error while terminating IEditController implementation.");
       if (processorComponent != nullptr)
           test(1,processorComponent->terminate() == kResultOk, "Error while terminating IComponent implementation.");
       return res;

    }
 

    virtual bool isSupported(std::string pluginPath) override
    {
        auto lib = LoadLib(pluginPath.c_str());
        if (lib == nullptr) return false;
        bool isSupp = LoadFunc<GetFactoryProc>(lib, "GetPluginFactory") != nullptr;
        UnloadLib(lib);
        return isSupp;
    }
};
REGISTER_TEST_SUITE(VST3TestSuite);

