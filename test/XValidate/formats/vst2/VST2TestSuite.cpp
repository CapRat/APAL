#include "../../IFormatTestSuite.hpp"
#include "tools/LibLoading.hpp"
#include "vst_def.hpp"
using namespace XPlug;
typedef AEffect* (*VSTPluginMain)(audioMasterCallback );

class VST2TestSuite :public FormatTestSuiteBase {
public:

    
    virtual std::string getFormatName() override
    {
        return "VST2";
    }

    virtual SucceedState run() override
    {
        size_t sampleSize =512;
        audioMasterCallback aMasterCallback = [](AEffect* effect, int32_t opCode, int32_t, intptr_t, void*, float)->intptr_t {
            switch (opCode) {
            case audioMasterProcessEvents:
                break;
            case audioMasterVersion :
                return 100;
            }
            return 0;
        };
        auto lib = LoadLib(data.pluginPath.c_str());
        if (lib == nullptr) return false;
        auto VSTPluginMain_fnc = LoadFunc<VSTPluginMain>(lib, "VSTPluginMain");
 
        auto effect = VSTPluginMain_fnc(aMasterCallback);
        test(1, effect != nullptr, "Error, couldnt create VST2 effect.");
        effect->dispatcher(effect, effOpen, NULL, NULL, nullptr, NULL);// Open Effect
        effect->dispatcher(effect, effMainsChanged, NULL, 1, nullptr, NULL); // Activate plugin (1 turn on , 0 turn off)
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
        //effect->dispatcher(effect,effCanDo,NULL, NULL, "receiveVstMidiEvent", NULL);
        //effect->dispatcher(effect, effCanDo, NULL, NULL, "sendVstMidiEvent", NULL);
        effect->dispatcher(effect, effClose, NULL, NULL, nullptr, NULL);// Close Effect
      

     
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