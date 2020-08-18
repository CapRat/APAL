#ifndef VST2_MODULE_HPP
#define VST2_MODULE_HPP
#include "tools/LibLoading.hpp"
#include "vst_def.hpp"
typedef AEffect* (*VSTPluginMain)(audioMasterCallback);

/**
 * @brief Simple Class, which is used to host vst2 Plugins.
 * Main task of this class is to call Pluginfunctions in a correct way and
 * expose an... kind of easy Interface. Like the other Module classes its an
 * quick and dirty class for tests. If its used more often, refractor it.
 */
class VST2Module
{
private:
  size_t samplesize = 0;
  VSTPluginMain VSTPluginMain_fnc = nullptr;
  float** inData = nullptr;
  float** outData = nullptr;
  audioMasterCallback aMasterCallback = nullptr;

public:
  // reference to the plugin library
  APAL::library_t pluginLib = nullptr;
  // the library represented as AEffect.
  AEffect* effect = nullptr;

  /**
   * @brief  Creates an VST Module. Also loads the library and Function, so when
   * an invalid path is given... its created.. but it shouldnt be used here.
   * TODO: Add exceptions here, so an invalid path throw an exception.
   * @param pluginPath path to vst2 binary.
   * @param cb the audioMasterCallback, which should be specified from host. If
   * not given an implementation which does nothing, except returning a 100 o
   * audioMasterVersion is taken.
   */
  inline VST2Module(std::string pluginPath, audioMasterCallback cb = nullptr)
  {
    if (cb == nullptr) {
      cb = [](AEffect*, int32_t opCode, int32_t, intptr_t, void*, float)
        -> intptr_t {
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
    pluginLib = APAL::LoadLib(pluginPath.c_str());
    VSTPluginMain_fnc =
      APAL::LoadFunc<VSTPluginMain>(pluginLib, "VSTPluginMain");
  }
  /**
   * @brief Calls the internal Free Method and unloads the library. So
   * everything is freed.
   */
  inline ~VST2Module()
  {
    this->free();
    APAL::UnloadLib(this->pluginLib);
  }

  /**
   * @brief initalizes the plugin with the required function calls.
   * @return returns true, if successful and false if not.
   */
  inline bool intialise()
  {
    effect = VSTPluginMain_fnc(aMasterCallback);
    effect->dispatcher(effect, effOpen, 0, 0, nullptr, 0); // Open Effect
    effect->dispatcher(effect,
                       effMainsChanged,
                       0,
                       1,
                       nullptr,
                       0); // Activate plugin (1 turn on , 0 turn off)
    return true;
  }
  /**
   * @brief Deinitalizes the plugin, with the required function calls.
   * @return returns true, if successful and false if not.
   */
  inline bool deinitialize()
  {
    effect->dispatcher(effect, effMainsChanged, 0, 0, nullptr, 0);
    effect->dispatcher(effect, effClose, 0, 0, nullptr, 0); // Close Effect
    return true;
  }
  /**
   * @brief Allocates Memory internal for the required Functions
   * @param _samplesize the size of each allocated array.
   * @return returns true, if successful and false if not.
   */
  inline bool allocate(size_t _samplesize)
  {
    this->free();
    this->samplesize = _samplesize;
    inData = new float*[effect->numInputs];
    outData = new float*[effect->numOutputs];
    for (int i = 0; i < effect->numInputs; i++)
      inData[i] = new float[this->samplesize];
    for (int i = 0; i < effect->numOutputs; i++)
      outData[i] = new float[this->samplesize];
    return true;
  }

  /**
   * @brief frees every allocated Memory. Clears nothing, if nothing is
   * allocated.
   * @return  returns true, if successful and false if not.
   */
  inline bool free()
  {
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
  /**
   * @brief Runs the plugin process function.
   */
  inline void run()
  {
    effect->processReplacing(effect, inData, outData, this->samplesize);
  }

  /**
   * @brief Sends an MidiMessage to the Plugin. (Receiving is done with the
   * given  audioMasterCallback callback function in the constructor.(
   * @param msg message to send.
   */
  inline void sendMidi(char msg[3])
  {
    if (effect->dispatcher(
          effect, effCanDo, 0, 0, (void*)"receiveVstMidiEvent", 0) == 1) {
      VstMidiEvent mEvent{ kVstMidiType,
                           sizeof(VstMidiEvent),
                           0,
                           0,
                           0,
                           0,
                           { msg[0], msg[1], msg[2], 0x0 },
                           0,
                           0,
                           0,
                           0 };
      VstEvent* mEventP = (VstEvent*)&mEvent;
      VstEvents ev{ 1, nullptr, { mEventP, NULL } };
      effect->dispatcher(effect, effProcessEvents, 0, 0, &ev, 0);
    }
  }
};

#endif //! VST2_MODULE_HPP