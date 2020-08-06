#ifndef VST3_MODULE_HPP
#define VST3_MODULE_HPP

#include <vst/ivstevents.h>
#include <vst/ivstaudioprocessor.h>
#include <vst/ivsteditcontroller.h>
#include "tools/LibLoading.hpp"
#include <functional>
#include <string>
#include <vector>
/**
 * @brief Implementation of the IEventList interfacem but without query ,addref and release functions, because it should not be needed.
 */
class EventList : public Steinberg::Vst::IEventList {
private:
    std::vector<Steinberg::Vst::Event> events;
public:
    inline virtual Steinberg::int32 getEventCount()override { return (Steinberg::int32)events.size(); }
    inline virtual ~EventList() {};
    inline virtual Steinberg::tresult getEvent(Steinberg::int32 index, Steinberg::Vst::Event& e) override {
        if (index < (Steinberg::int32)events.size()) {
            e = events[index];
            return Steinberg::kResultOk;
        }
        return Steinberg::kResultFalse;
    }
    inline virtual Steinberg::tresult addEvent(Steinberg::Vst::Event& e) override {
        this->events.push_back(e);
        return Steinberg::kResultOk;
    }
    inline virtual void clear() { this->events.clear(); }
    //dont implement that stupid COM Interface, cause it should not be used here...
    inline virtual  Steinberg::tresult queryInterface(const  Steinberg::TUID , void** ) override { return Steinberg::kNoInterface; }
    inline  virtual  Steinberg::uint32  addRef() override { return 0; }
    inline virtual  Steinberg::uint32  release() override { return 0; }
};


/**
 * @brief Simple class to handle allocating and freeing of Memory for ProcessData struct.
 */
struct ProcessDataClass :public Steinberg::Vst::ProcessData {
protected:
    std::vector<Steinberg::Vst::AudioBusBuffers> inputAudioBusBuffers;
    std::vector<Steinberg::Vst::AudioBusBuffers> outputAudioBusBuffers;
    EventList inputEventsClass;
    EventList outputEventsClass;
    // Just gets the numbers of channels in an Arrangement. (Hemmingweight)
    static inline size_t getNumberOfChannel(Steinberg::Vst::SpeakerArrangement arr)
    {
        size_t number = 0;
        while (arr)
        {
            if (arr & (Steinberg::Vst::SpeakerArrangement)1) ++number;
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
     * @param numSamples Numbers of Samples in one buffer per bus.
    */
    inline ProcessDataClass(std::vector<Steinberg::Vst::SpeakerArrangement> _inputChannels, std::vector<Steinberg::Vst::SpeakerArrangement> _outputChannels, Steinberg::Vst::SymbolicSampleSizes _sampleSize, size_t _numSamples) {
        using namespace Steinberg;
        using namespace Vst;
        this->processMode = ProcessModes::kRealtime;
        this->symbolicSampleSize = _sampleSize;
        this->processContext = nullptr;
        this->numSamples = (Steinberg::int32)_numSamples;
        this->numInputs = _inputChannels.size();
        this->numOutputs = _outputChannels.size();
        inputAudioBusBuffers = std::vector<AudioBusBuffers>(this->numInputs);
        outputAudioBusBuffers = std::vector<AudioBusBuffers>(this->numOutputs);
        this->inputs = inputAudioBusBuffers.data();
        this->outputs = outputAudioBusBuffers.data();
        this->inputEvents = &inputEventsClass;
        this->outputEvents = &outputEventsClass;
        this->inputParameterChanges = nullptr;
        this->outputParameterChanges = nullptr;
        auto allocator = [this](std::vector<AudioBusBuffers>& buffers, std::vector<SpeakerArrangement>& channelConfig) {
            for (size_t i = 0; i < buffers.size(); i++) {
                buffers[i].numChannels = getNumberOfChannel(channelConfig[i]);
                buffers[i].silenceFlags = 0;
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
        allocator(inputAudioBusBuffers, _inputChannels);
        allocator(outputAudioBusBuffers, _outputChannels);
    }
    /**
     * @brief Deletes any allocated Memory.
     */
    inline ~ProcessDataClass() {
        using namespace Steinberg;
        using namespace Vst;
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
#define WARN_IF_AND_THAN(cond, msg,andthen) if(cond) { lastWarning = msg; andthen}
#define WARN_IF(cond, msg) WARN_IF_AND_THAN(cond, msg,return false;)
#define ERROR_IF_AND_THAN(cond, msg,andthen) if(cond) { lastError = msg; andthen}
#define ERROR_IF(cond, msg)ERROR_IF_AND_THAN(cond,msg,return false;)
class VST3Module {
private:
    std::string lastError="";
    std::string lastWarning="";
    XPlug::library_t pluginLibrary; // The pluginlibrary
    GetFactoryProc plugLoadFunction; // FactoryLoad Function
    void* initfnc; // functionpointer to init function
    void* exitfnc; // functionpointer to init function
   

    Steinberg::Vst::IComponent* processorComponent = nullptr;
    Steinberg::Vst::IEditController* editController = nullptr;
    Steinberg::Vst::IAudioProcessor* audioProcessor = nullptr;
    Steinberg::IPluginFactory* factory = nullptr;
    Steinberg::IPluginFactory2* factory2 = nullptr;
    Steinberg::IPluginFactory3* factory3 = nullptr;
    std::unique_ptr<ProcessDataClass> pData = nullptr;

public:
    std::function<bool()>runInitFncLampda; // Function to call init function crossplattform. Returns Value from InitDll/InitModule/bundleEntry
    std::function<bool()>runExitFncLampda; // Function to call exit function crossplattform.Returns Value from  ExitDll/ExitModule/bundleExit
    inline bool hasInitAndExitFunctions() { return initfnc != nullptr && exitfnc != nullptr; }

    inline VST3Module(std::string pluginPath) {
        using namespace XPlug;
        using namespace Steinberg;
        using namespace Vst;
        GetError();
        this->pluginLibrary = LoadLib(pluginPath.c_str());
        ERROR_IF_AND_THAN(this->pluginLibrary == nullptr, "Error, while loading VST3 binary, with Message :" + GetErrorStr(), return;)

            this->plugLoadFunction = LoadFunc<GetFactoryProc >(this->pluginLibrary, "GetPluginFactory");
        ERROR_IF_AND_THAN(this->plugLoadFunction == nullptr, "Error, while loading VST3 function, with Message :" + GetErrorStr(), return;)
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
        WARN_IF_AND_THAN(hasInitAndExitFunctions(), "No initialisation functions given, for VST3 Module." + GetErrorStr(), return;); 
        if(this->runInitFncLampda!=nullptr)
            this->runInitFncLampda();

    }
    inline ~VST3Module() {
        if(this->runExitFncLampda!=nullptr)
            this->runExitFncLampda();
        XPlug::UnloadLib(this->pluginLibrary);
    }
    inline bool initialize() {
        using namespace Steinberg;
        using namespace Vst;
        this->factory = plugLoadFunction();
        ERROR_IF(this->factory == nullptr, "Error while loading factory. Factory is NULL");
        ERROR_IF((this->factory->countClasses() < 2), "Error There are less then 2 classes in Factory.There must be at least one IAudioProcessor and one IEditController class.");
        PFactoryInfo info;
        ERROR_IF((this->factory->getFactoryInfo(&info) != kResultOk), "Cant get FactoryInfo from factory");
        this->factory->queryInterface(IPluginFactory3::iid, (void**)&factory3);
        this->factory->queryInterface(IPluginFactory2::iid, (void**)&factory2);
        for (Steinberg::int32 i = 0; i < this->factory->countClasses(); i++) {
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
                ERROR_IF((this->factory->getFactoryInfo(&facInfo) != kResultOk), "Cant get FactoryInfo from factory");
                Steinberg::PClassInfo2 ci2(ci.cid, ci.cardinality, ci.category, ci.name, 0, NULL, facInfo.vendor, "0.0.0.0", "VST 3.0.0");
                ci3.fromAscii(ci2);
            }

            if (strcmp(ci3.category, kVstAudioEffectClass) == 0) {//Class is an Audio Effect Processor
                ERROR_IF(((this->factory->createInstance(ci3.cid, Vst::IComponent::iid, (void**)&processorComponent)
                    != kResultOk)), "Error, could not create IComponentobject successfully");
                std::string x;
                int j = 0;
                while (ci3.name[j] != 0) {
                    x.push_back(static_cast<char>(ci3.name[j++]));
                }
                ERROR_IF((processorComponent->initialize(nullptr) != kResultOk), std::string("Error while initialising Component ") + x);
                TUID controllerCID;
                ERROR_IF(processorComponent->getControllerClassId(controllerCID) != kResultTrue, "Error,while getting ControllerID from IComponent.");
                auto controllerFUID = FUID::fromTUID(controllerCID);

                ERROR_IF(!controllerFUID.isValid(), "Error, ControllerID from IComponent is not valid.");
                ERROR_IF((this->factory->createInstance(controllerCID, Vst::IEditController::iid, (void**)&editController)
                    != kResultOk), "Error while creating EditController from Factory.");
                ERROR_IF(editController->initialize(nullptr) != kResultOk, "Error while initializing EditController");
                ERROR_IF((processorComponent->queryInterface(Vst::IAudioProcessor::iid, (void**)&audioProcessor) != kResultOk), "Error, while retreiving the AudioInterface");
            }
            else {
                continue;
            }
        }
        return this->audioProcessor!=nullptr && this->editController!=nullptr;
    }

    inline bool deinitalize() {
        if (editController != nullptr)
            ERROR_IF(editController->terminate() != Steinberg::kResultOk, "Error while terminating IEditController implementation.");
        if (processorComponent != nullptr)
            ERROR_IF(processorComponent->terminate() != Steinberg::kResultOk, "Error while terminating IEditController implementation.");
        return true;
    }

    inline bool allocateData(size_t audioPortBufferSize) {
        using namespace Steinberg;
        using namespace Vst;
        std::vector<SpeakerArrangement> inputs(this->processorComponent->getBusCount(MediaTypes::kAudio, BusDirections::kInput));
        std::vector<SpeakerArrangement> outputs(this->processorComponent->getBusCount(MediaTypes::kAudio, BusDirections::kOutput));
        SymbolicSampleSizes size = this->audioProcessor->canProcessSampleSize(kSample64) ? kSample64 : kSample32;
        size_t i = 0;
        for (auto& sArr : inputs)
            ERROR_IF(this->audioProcessor->getBusArrangement(BusDirections::kInput, i++, sArr) != kResultOk, "Error, Couldnt get Busarrangement");
        i = 0;
        for (auto& sArr : outputs)
            ERROR_IF(this->audioProcessor->getBusArrangement(BusDirections::kOutput, i++, sArr) != kResultOk, "Error, Couldnt get Busarrangement");
        pData=std::make_unique<ProcessDataClass>(inputs, outputs,size, audioPortBufferSize);
        return true;
    }

    inline bool run() {
        ERROR_IF(this->audioProcessor->setProcessing(true)!=Steinberg::kResultOk,"Error, while set Processing to true");
        ERROR_IF(this->audioProcessor->process(*(this->pData.get())) != Steinberg::kResultOk, "Error, because processing not returned valid State");
        ERROR_IF(this->audioProcessor->setProcessing(false) != Steinberg::kResultOk, "Error, while set Processing to false");
        return true;
    }
    inline std::string getLastError() { return lastError; }
    inline std::string getLastWarning() { return lastWarning; }
    inline ProcessDataClass* getDataBuffer() { return pData.get(); }
};


#endif //! VST3_MODULE_HPP