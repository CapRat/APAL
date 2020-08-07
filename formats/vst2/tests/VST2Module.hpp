#ifndef VST2_MODULE_HPP
#define VST2_MODULE_HPP
#include "tools/LibLoading.hpp"
#include "vst_def.hpp"
typedef AEffect* (*VSTPluginMain)(audioMasterCallback);
/*
struct VST2AudioData {
    float** inData = nullptr;
    float** outData = nullptr;
    size_t inDataSize = 0;
    size_t outDataSize = 0;
    size_t sampleCount = 0;

    inline ~VST2AudioData() { this->free(); }
    inline bool allocate() {
        this->free();
        inData = new float* [inDataSize];
        outData = new float* [outDataSize];
        for (int i = 0; i < inDataSize; i++)
            inData[i] = new float[sampleCount];
        for (int i = 0; i < outDataSize; i++)
            outData[i] = new float[sampleCount];
        return true;
    }

    inline bool free() {
        if (inData != nullptr) {
            for (int i = 0; i < inDataSize; i++)
                if (inData[i] != nullptr)
                    delete[] inData[i];
            delete[] inData;
        }
        if (outData != nullptr) {
            for (int i = 0; i < outDataSize; i++)
                if (outData[i] != nullptr)
                    delete[] outData[i];
            delete[] outData;
        }
        return true;
    }
};*/

class VST2Module {
private:
    size_t samplesize = 0;
    VSTPluginMain VSTPluginMain_fnc = nullptr;
    float** inData = nullptr;
    float** outData = nullptr;
    audioMasterCallback aMasterCallback = nullptr;
public:
    XPlug::library_t pluginLib = nullptr;
    AEffect* effect=nullptr;

    inline VST2Module(std::string pluginPath, audioMasterCallback cb =nullptr) {
        if (cb == nullptr) {
            cb = [](AEffect* , int32_t opCode, int32_t, intptr_t, void*, float)->intptr_t {
                switch (opCode) {
                case audioMasterProcessEvents:
                    break;
                case audioMasterVersion:
                    return 100;
                }
                return 0;
            };
        }

        aMasterCallback = cb;
        pluginLib= XPlug::LoadLib(pluginPath.c_str());
        VSTPluginMain_fnc = XPlug::LoadFunc<VSTPluginMain>(pluginLib, "VSTPluginMain");
    }
    inline ~VST2Module() {
        this->free();
        XPlug::UnloadLib(this->pluginLib);
    }
    inline bool intialise() {
        effect = VSTPluginMain_fnc(aMasterCallback);
        effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0);// Open Effect
        effect->dispatcher(effect, effMainsChanged, 0, 1, nullptr, 0); // Activate plugin (1 turn on , 0 turn off)
        return true;
    }
    inline bool deinitialize() {
        effect->dispatcher(effect, effMainsChanged, 0, 0, nullptr, 0);
        effect->dispatcher(effect, effClose, 0, 0, nullptr, 0);// Close Effect
        return true;
    }
    inline bool allocate(size_t _samplesize) {
        this->free();
        this->samplesize = _samplesize;
        inData = new float* [effect->numInputs];
        outData = new float* [effect->numOutputs];
        for (int i = 0; i < effect->numInputs; i++)
            inData[i] = new float[this->samplesize ];
        for (int i = 0; i < effect->numOutputs; i++)
            outData[i] = new float[this->samplesize];
        return true;
    }

    inline bool free() {
        if (inData != nullptr) {
            for (int i = 0; i < effect->numInputs; i++)
                if (inData[i] != nullptr)
                    delete[] inData[i];
            delete[] inData;
            inData = nullptr;
        }
        if (outData != nullptr) {
            for (int i = 0; i < effect->numOutputs; i++)
                if (outData[i] != nullptr)
                    delete[] outData[i];
            delete[] outData;
            outData = nullptr;
        }
        return true;
    }

    inline void run() {
        effect->processReplacing(effect, inData, outData, this->samplesize);
    }
    inline void sendMidi(char msg[3]) {
        if (effect->dispatcher(effect, effCanDo, 0, 0, (void*)"receiveVstMidiEvent", 0) == 1) {
            VstMidiEvent mEvent{ kVstMidiType,sizeof(VstMidiEvent),0,0,0,0,{ msg[0],msg[1],msg[2],0x0 },0,0,0,0 };
            VstEvent* mEventP = (VstEvent*)&mEvent;
            VstEvents ev{ 1,nullptr,{ mEventP,NULL } };
            effect->dispatcher(effect, effProcessEvents, 0, 0, &ev, 0);
        }
    }

};

#endif //! VST2_MODULE_HPP