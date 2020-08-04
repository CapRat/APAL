#include "../../IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include "vst_def.hpp"
using namespace XPlug;
typedef AEffect* (*VSTPluginMain)(audioMasterCallback);

class VST2TestSuite :public FormatTestSuiteBase {
public:


    virtual std::string getFormatName() override
    {
        return "VST2";
    }

    virtual SucceedState run() override
    {
        size_t sampleSize = 512;
        audioMasterCallback aMasterCallback = [](AEffect* effect, int32_t opCode, int32_t, intptr_t, void*, float)->intptr_t {
            switch (opCode) {
            case audioMasterProcessEvents:
                break;
            case audioMasterVersion:
                return 100;
            }
            return 0;
        };
        auto lib = LoadLib(data.pluginPath.c_str());
        if (lib == nullptr) return false;
        auto VSTPluginMain_fnc = LoadFunc<VSTPluginMain>(lib, "VSTPluginMain");

        auto effect = VSTPluginMain_fnc(aMasterCallback);
        test(1, effect != nullptr, "Error, couldnt create VST2 effect.");
        effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0);// Open Effect
        effect->dispatcher(effect, effMainsChanged, 0, 1, nullptr, 0); // Activate plugin (1 turn on , 0 turn off)
        // Audio processing

        float** inData = new float* [effect->numInputs];
        float** outData = new float* [effect->numOutputs];
        for (int i = 0; i < effect->numInputs; i++)
            inData[i] = new float[sampleSize];
        for (int i = 0; i < effect->numOutputs; i++)
            outData[i] = new float[sampleSize];
        effect->processReplacing(effect, inData, outData, sampleSize);
        //cleanup allocated arrays.
        for (int i = 0; i < effect->numInputs; i++)
            delete[] inData[i];
        for (int i = 0; i < effect->numOutputs; i++)
            delete[] outData[i];
        delete[] inData;
        delete[] outData;
        //MIDI TEST
        if (effect->dispatcher(effect, effCanDo, 0, 0, (void*)"receiveVstMidiEvent", 0) == 1){
            VstMidiEvent mEvent{ kVstMidiType,sizeof(VstMidiEvent),0,0,0,0,{ 0x1,0x2,0x3,0x0 },0,0,0,0 };
            VstEvent* mEventP = (VstEvent*)&mEvent;
            VstEvents ev{ 1,nullptr,{ mEventP,NULL } };
            effect->dispatcher(effect, effProcessEvents, 0, 0, &ev, 0);
        }
        effect->dispatcher(effect, effCanDo, 0, 0, (void*)"sendVstMidiEvent", 0);
        effect->dispatcher(effect, effMainsChanged, 0, 0, nullptr, 0);
        effect->dispatcher(effect, effClose, 0, 0, nullptr, 0);// Close Effect
      

     
        return TEST_SUCCEEDED;
    }

    virtual SucceedState runPerformance() override
    {
        return SucceedState();
    }
    virtual bool isSupported(std::string pluginPath) override
    {
        auto lib = LoadLib(pluginPath.c_str());
        if (lib == nullptr) return false;
        bool isSupp =  LoadFunc<VSTPluginMain>(lib, "VSTPluginMain") != nullptr;
        UnloadLib(lib);
        return isSupp;
    }

};
REGISTER_TEST_SUITE(VST2TestSuite);