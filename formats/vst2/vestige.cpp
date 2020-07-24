#include "interfaces/IPlugin.hpp"
#include "vst_def.hpp"

//#include "C:\Users\Benjamin\Downloads\vst_sdk2_4_rev1\vstsdk2.4\pluginterfaces\vst2.x\aeffectx.h"
#include <GlobalData.hpp>
#include <interfaces/IPlugin.hpp>
#include <algorithm>
#include <cstring>
#include <tools/PortHandling.hpp>
using namespace XPlug;
/**
 * @brief Struct for  use in the vst2 implementation. This struct is stored in the AEffect->user pointer. So it can be casted to this struct.
 * If the VST2 implementation needs to hold data, pack it in here.
 */
struct VST2ImplementationData {
    audioMasterCallback audioMaster;
    IPlugin* plug;
};

float vst_getParameter(const int32_t index)
{
    /*const ParameterRanges& ranges(fPlugin.getParameterRanges(index));
    return ranges.getNormalizedValue(fPlugin.getParameterValue(index));*/
    return 0.0;
}

void vst_setParameter(const int32_t index, const float value)
{
    /*
    const uint32_t hints(fPlugin.getParameterHints(index));
    const ParameterRanges& ranges(fPlugin.getParameterRanges(index));

    // TODO figure out how to detect kVstParameterUsesIntegerMinMax host support, and skip normalization
    float realValue = ranges.getUnnormalizedValue(value);

    if (hints & kParameterIsBoolean)
    {
        const float midRange = ranges.min + (ranges.max - ranges.min) / 2.0f;
        realValue = realValue > midRange ? ranges.max : ranges.min;
    }

    if (hints & kParameterIsInteger)
    {
        realValue = std::round(realValue);
    }

    fPlugin.setParameterValue(index, realValue);

#if DISTRHO_PLUGIN_HAS_UI
    if (fVstUI != nullptr)
        setParameterValueFromPlugin(index, realValue);
#endif
}

void vst_processReplacing(const float** const inputs, float** const outputs, const int32_t sampleFrames)
{
    if (sampleFrames <= 0)
    {
        updateParameterOutputsAndTriggers();
        return;
    }

    if (!fPlugin.isActive())
    {
        // host has not activated the plugin yet, nasty!
        vst_dispatcher(effMainsChanged, 0, 1, nullptr, 0.0f);
    }

#if DISTRHO_PLUGIN_WANT_TIMEPOS
    static const int kWantVstTimeFlags(kVstTransportPlaying | kVstPpqPosValid | kVstTempoValid | kVstTimeSigValid);

    if (const VstTimeInfo* const vstTimeInfo = (const VstTimeInfo*)hostCallback(audioMasterGetTime, 0, kWantVstTimeFlags))
    {
        fTimePosition.frame = vstTimeInfo->samplePos;
        fTimePosition.playing = (vstTimeInfo->flags & kVstTransportPlaying);
        fTimePosition.bbt.valid = ((vstTimeInfo->flags & kVstTempoValid) != 0 || (vstTimeInfo->flags & kVstTimeSigValid) != 0);

        // ticksPerBeat is not possible with VST
        fTimePosition.bbt.ticksPerBeat = 960.0;

        if (vstTimeInfo->flags & kVstTempoValid)
            fTimePosition.bbt.beatsPerMinute = vstTimeInfo->tempo;
        else
            fTimePosition.bbt.beatsPerMinute = 120.0;

        if (vstTimeInfo->flags & (kVstPpqPosValid | kVstTimeSigValid))
        {
            const double ppqPos = std::abs(vstTimeInfo->ppqPos);
            const int    ppqPerBar = vstTimeInfo->timeSigNumerator * 4 / vstTimeInfo->timeSigDenominator;
            const double barBeats = (std::fmod(ppqPos, ppqPerBar) / ppqPerBar) * vstTimeInfo->timeSigNumerator;
            const double rest = std::fmod(barBeats, 1.0);

            fTimePosition.bbt.bar = static_cast<int32_t>(ppqPos) / ppqPerBar + 1;
            fTimePosition.bbt.beat = static_cast<int32_t>(barBeats - rest + 0.5) + 1;
            fTimePosition.bbt.tick = static_cast<int32_t>(rest * fTimePosition.bbt.ticksPerBeat + 0.5);
            fTimePosition.bbt.beatsPerBar = vstTimeInfo->timeSigNumerator;
            fTimePosition.bbt.beatType = vstTimeInfo->timeSigDenominator;

            if (vstTimeInfo->ppqPos < 0.0)
            {
                --fTimePosition.bbt.bar;
                fTimePosition.bbt.beat = vstTimeInfo->timeSigNumerator - fTimePosition.bbt.beat + 1;
                fTimePosition.bbt.tick = int(fTimePosition.bbt.ticksPerBeat) - fTimePosition.bbt.tick - 1;
            }
        }
        else
        {
            fTimePosition.bbt.bar = 1;
            fTimePosition.bbt.beat = 1;
            fTimePosition.bbt.tick = 0;
            fTimePosition.bbt.beatsPerBar = 4.0f;
            fTimePosition.bbt.beatType = 4.0f;
        }

        fTimePosition.bbt.barStartTick = fTimePosition.bbt.ticksPerBeat * fTimePosition.bbt.beatsPerBar * (fTimePosition.bbt.bar - 1);

        fPlugin.setTimePosition(fTimePosition);
    }
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    fPlugin.run(inputs, outputs, sampleFrames, fMidiEvents, fMidiEventCount);
    fMidiEventCount = 0;
#else
    fPlugin.run(inputs, outputs, sampleFrames);
#endif

    updateParameterOutputsAndTriggers();*/
}



// -------------------------------------------------------------------

static intptr_t vst_dispatcherCallback(AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void* ptr, float opt)
{
    // handle base opcodes
    switch (opcode) {
        /**
         * Opens the Effect .
         */
    case effOpen:

        /*    audioMasterCallback audioMaster = (audioMasterCallback)obj->audioMaster;

            d_lastBufferSize = audioMaster(effect, audioMasterGetBlockSize, 0, 0, nullptr, 0.0f);
            d_lastSampleRate = audioMaster(effect, audioMasterGetSampleRate, 0, 0, nullptr, 0.0f);

            // some hosts are not ready at this point or return 0 buffersize/samplerate
            if (d_lastBufferSize == 0)
                d_lastBufferSize = 2048;
            if (d_lastSampleRate <= 0.0)
                d_lastSampleRate = 44100.0;

            obj->plugin = new PluginVst(audioMaster, effect);
            return 1;
        }*/
        return 0;

    case effClose:
        /**
         * Close effect.
         */
        return 0;

    case effGetParamLabel:
        /*   if (ptr != nullptr && index < static_cast<int32_t>(plugin.getParameterCount()))
           {
               DISTRHO_NAMESPACE::strncpy((char*)ptr, plugin.getParameterUnit(index), 8);
               return 1;
           }*/
        return 0;

    case effGetParamName:
        /* if (ptr != nullptr && index < static_cast<int32_t>(plugin.getParameterCount()))
         {
             const String& shortName(plugin.getParameterShortName(index));
             if (shortName.isNotEmpty())
                 DISTRHO_NAMESPACE::strncpy((char*)ptr, shortName, 16);
             else
                 DISTRHO_NAMESPACE::strncpy((char*)ptr, plugin.getParameterName(index), 16);
             return 1;
         }*/
        return 0;

    case effGetParameterProperties:
        /*  if (ptr != nullptr && index < static_cast<int32_t>(plugin.getParameterCount()))
          {
              if (VstParameterProperties* const properties = (VstParameterProperties*)ptr)
              {
                  memset(properties, 0, sizeof(VstParameterProperties));

                  const uint32_t hints = plugin.getParameterHints(index);

                  if (hints & kParameterIsOutput)
                      return 1;

                  if (hints & kParameterIsBoolean)
                  {
                      properties->flags |= kVstParameterIsSwitch;
                  }

                  if (hints & kParameterIsInteger)
                  {
                      properties->flags |= kVstParameterUsesIntegerMinMax;
                      const ParameterRanges& ranges(plugin.getParameterRanges(index));

                      properties->minInteger = static_cast<int32_t>(ranges.min);
                      properties->maxInteger = static_cast<int32_t>(ranges.max);
                  }

                  if (hints & kParameterIsLogarithmic)
                  {
                      properties->flags |= kVstParameterCanRamp;
                  }

                  return 1;
              }
          }*/
        return 0;

    case effGetPlugCategory:
        /*#if DISTRHO_PLUGIN_IS_SYNTH
                return kPlugCategSynth;
        #else
                return kPlugCategEffect;
        #endif*/

    case effGetEffectName:
        //return GlobalData().getPlugin(0)->getPluginInfo()->
        ptr = static_cast<void*>(const_cast<char*>(GlobalData().getPlugin(0)->getInfoComponent()->getPluginName().data()));
        //std::strncpy((char*)ptr, GlobalData().getPlugin(0)->getPluginInfo()->name.c_str(), 32);
        return 1;
        /*if (char* const cptr = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(cptr, plugin.getName(), 32);
            return 1;
        }*/
        // return 0;

    case effGetVendorString:
        ptr = static_cast<void*>(const_cast<char*>(GlobalData().getPlugin(0)->getInfoComponent()->getCreatorName().data()));
        // std::strncpy((char*)ptr, GlobalData().getPlugin(0)->getPluginInfo()->creater.c_str(), 32);
        /*if (char* const cptr = (char*)ptr)
         {
             DISTRHO_NAMESPACE::strncpy(cptr, plugin.getMaker(), 32);
             return 1;
         }*/
        return 0;

    case effGetProductString:
        ptr = static_cast<void*>(const_cast<char*>(GlobalData().getPlugin(0)->getInfoComponent()->getPluginDescription().data()));
        // std::strncpy((char*)ptr, GlobalData().getPlugin(0)->getPluginInfo()->description.c_str(), 32);
        /*if (char* const cptr = (char*)ptr)
          {
              DISTRHO_NAMESPACE::strncpy(cptr, plugin.getLabel(), 32);
              return 1;
          }*/
        return 0;

    case effGetVendorVersion:
        // return plugin.getVersion();

    case effGetVstVersion:
        return kVstVersion;

    case effGetProgram:
        return 0;

    case effSetProgramName:
        /*
        if (char* const programName = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(fProgramName, programName, 32);
            return 1;
        }*/
        break;

    case effGetProgramName:
        /*if (char* const programName = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(programName, fProgramName, 24);
            return 1;
        }*/
        break;

    case effGetProgramNameIndexed:
        /* if (char* const programName = (char*)ptr)
         {
             DISTRHO_NAMESPACE::strncpy(programName, fProgramName, 24);
             return 1;
         }*/
        break;

    case effGetParamDisplay:
        /*  if (ptr != nullptr && index < static_cast<int32_t>(fPlugin.getParameterCount()))
          {
              const uint32_t hints = fPlugin.getParameterHints(index);
              float value = fPlugin.getParameterValue(index);

              if (hints & kParameterIsBoolean)
              {
                  const ParameterRanges& ranges(fPlugin.getParameterRanges(index));
                  const float midRange = ranges.min + (ranges.max - ranges.min) / 2.0f;

                  value = value > midRange ? ranges.max : ranges.min;
              }
              else if (hints & kParameterIsInteger)
              {
                  value = std::round(value);
              }

              const ParameterEnumerationValues& enumValues(fPlugin.getParameterEnumValues(index));

              for (uint8_t i = 0; i < enumValues.count; ++i)
              {
                  if (d_isNotEqual(value, enumValues.values[i].value))
                      continue;

                  DISTRHO_NAMESPACE::strncpy((char*)ptr, enumValues.values[i].label.buffer(), 24);
                  return 1;
              }

              if (hints & kParameterIsInteger)
                  DISTRHO_NAMESPACE::snprintf_iparam((char*)ptr, (int32_t)value, 24);
              else
                  DISTRHO_NAMESPACE::snprintf_param((char*)ptr, value, 24);

              return 1;
          }*/
        break;

    case effSetSampleRate:
        // fPlugin.setSampleRate(opt, true);
        /* if (fVstUI != nullptr)
             fVstUI->setSampleRate(opt);*/
        break;

    case effSetBlockSize:
        //fPlugin.setBufferSize(value, true);
        break;

    case effMainsChanged:
        /*       if (value != 0)
               {
       #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
                   fMidiEventCount = 0;

                   // tell host we want MIDI events
                   hostCallback(audioMasterWantMidi);
       #endif

                   // deactivate for possible changes
                   fPlugin.deactivateIfNeeded();

                   // check if something changed
                   const uint32_t bufferSize = static_cast<uint32_t>(hostCallback(audioMasterGetBlockSize));
                   const double   sampleRate = static_cast<double>(hostCallback(audioMasterGetSampleRate));

                   if (bufferSize != 0)
                       fPlugin.setBufferSize(bufferSize, true);

                   if (sampleRate != 0.0)
                       fPlugin.setSampleRate(sampleRate, true);

                   fPlugin.activate();
               }
               else
               {
                   fPlugin.deactivate();
               }*/
        break;

    case effEditGetRect:
        /*DISTRHO_PLUGIN_HAS_UI
          if (fVstUI != nullptr)
           {
               fVstRect.right = fVstUI->getWidth();
               fVstRect.bottom = fVstUI->getHeight();
           }
           else
           {
               d_lastUiSampleRate = fPlugin.getSampleRate();

               // TODO
               const float scaleFactor = 1.0f;

               UIExporter tmpUI(nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, scaleFactor, fPlugin.getInstancePointer());
               fVstRect.right = tmpUI.getWidth();
               fVstRect.bottom = tmpUI.getHeight();
               tmpUI.quit();
           }
           *(ERect**)ptr = &fVstRect;
           return 1;*/

    case effEditOpen:
        /* DISTRHO_PLUGIN_HAS_UI
        delete fVstUI; // hosts which don't pair effEditOpen/effEditClose calls (Minihost Modular)
        fVstUI = nullptr;
        {
# if DISTRHO_OS_MAC
            if (!fUsingNsView)
            {
                d_stderr("Host doesn't support hasCockosViewAsConfig, cannot use UI");
                return 0;
            }
# endif
            d_lastUiSampleRate = fPlugin.getSampleRate();

            // TODO
            const float scaleFactor = 1.0f;

            fVstUI = new UIVst(fAudioMaster, fEffect, this, &fPlugin, (intptr_t)ptr, scaleFactor);

# if DISTRHO_PLUGIN_WANT_FULL_STATE
            // Update current state from plugin side
            for (StringMap::const_iterator cit = fStateMap.begin(), cite = fStateMap.end(); cit != cite; ++cit)
            {
                const String& key = cit->first;
                fStateMap[key] = fPlugin.getState(key);
            }
# endif

# if DISTRHO_PLUGIN_WANT_STATE
            // Set state
            for (StringMap::const_iterator cit = fStateMap.begin(), cite = fStateMap.end(); cit != cite; ++cit)
            {
                const String& key = cit->first;
                const String& value = cit->second;

                fVstUI->setStateFromPlugin(key, value);
            }
# endif
            for (uint32_t i = 0, count = fPlugin.getParameterCount(); i < count; ++i)
                setParameterValueFromPlugin(i, fPlugin.getParameterValue(i));

            fVstUI->idle();
            return 1;
        }
        */
        break;

    case effEditClose:
        /*DISTRHO_PLUGIN_HAS_UI
      if (fVstUI != nullptr)
        {
            delete fVstUI;
            fVstUI = nullptr;
            return 1;
        }*/
        break;

        //case effIdle:
    case effEditIdle:
        /*DISTRHO_PLUGIN_HAS_UI
        if (fVstUI != nullptr)
             fVstUI->idle();*/
        break;

    case effEditKeyDown:
        /*DISTRHO_PLUGIN_HAS_UI
        if (fVstUI != nullptr)
             return fVstUI->handlePluginKeyEvent(true, index, value);*/
        break;

    case effEditKeyUp:
        /*DISTRHO_PLUGIN_HAS_UI
        if (fVstUI != nullptr)
              return fVstUI->handlePluginKeyEvent(false, index, value);*/
        break;

    case effGetChunk: {
        /* STATE BLAH
        if (ptr == nullptr)
            return 0;

        if (fStateChunk != nullptr)
        {
            delete[] fStateChunk;
            fStateChunk = nullptr;
        }

        const uint32_t paramCount = fPlugin.getParameterCount();

        if (fPlugin.getStateCount() == 0 && paramCount == 0)
        {
            fStateChunk = new char[1];
            fStateChunk[0] = '\0';
            ret = 1;
        }
        else
        {

            // Update current state
            for (StringMap::const_iterator cit = fStateMap.begin(), cite = fStateMap.end(); cit != cite; ++cit)
            {
                const String& key = cit->first;
                fStateMap[key] = fPlugin.getState(key);
            }


            String chunkStr;

            for (StringMap::const_iterator cit = fStateMap.begin(), cite = fStateMap.end(); cit != cite; ++cit)
            {
                const String& key = cit->first;
                const String& value = cit->second;

                // join key and value
                String tmpStr;
                tmpStr = key;
                tmpStr += "\xff";
                tmpStr += value;
                tmpStr += "\xff";

                chunkStr += tmpStr;
            }

            if (paramCount != 0)
            {
                // add another separator
                chunkStr += "\xff";

                // temporarily set locale to "C" while converting floats
                const ScopedSafeLocale ssl;

                for (uint32_t i = 0; i < paramCount; ++i)
                {
                    if (fPlugin.isParameterOutputOrTrigger(i))
                        continue;

                    // join key and value
                    String tmpStr;
                    tmpStr = fPlugin.getParameterSymbol(i);
                    tmpStr += "\xff";
                    tmpStr += String(fPlugin.getParameterValue(i));
                    tmpStr += "\xff";

                    chunkStr += tmpStr;
                }
            }

            const std::size_t chunkSize(chunkStr.length() + 1);

            fStateChunk = new char[chunkSize];
            std::memcpy(fStateChunk, chunkStr.buffer(), chunkStr.length());
            fStateChunk[chunkSize - 1] = '\0';

            for (std::size_t i = 0; i < chunkSize; ++i)
            {
                if (fStateChunk[i] == '\xff')
                    fStateChunk[i] = '\0';
            }

            ret = chunkSize;
        }

        *(void**)ptr = fStateChunk;
        return ret;
        */
    }

    case effSetChunk: {
        /* STATE BLAH
        if (value <= 1 || ptr == nullptr)
            return 0;

        const size_t chunkSize = static_cast<size_t>(value);

        const char* key = (const char*)ptr;
        const char* value = nullptr;
        size_t size, bytesRead = 0;

        while (bytesRead < chunkSize)
        {
            if (key[0] == '\0')
                break;

            size = std::strlen(key) + 1;
            value = key + size;
            bytesRead += size;

            setStateFromUI(key, value);

# if DISTRHO_PLUGIN_HAS_UI
            if (fVstUI != nullptr)
                fVstUI->setStateFromPlugin(key, value);
# endif

            // get next key
            size = std::strlen(value) + 1;
            key = value + size;
            bytesRead += size;
        }

        const uint32_t paramCount = fPlugin.getParameterCount();

        if (bytesRead + 4 < chunkSize && paramCount != 0)
        {
            ++key;
            float fvalue;

            // temporarily set locale to "C" while converting floats
            const ScopedSafeLocale ssl;

            while (bytesRead < chunkSize)
            {
                if (key[0] == '\0')
                    break;

                size = std::strlen(key) + 1;
                value = key + size;
                bytesRead += size;

                // find parameter with this symbol, and set its value
                for (uint32_t i = 0; i < paramCount; ++i)
                {
                    if (fPlugin.isParameterOutputOrTrigger(i))
                        continue;
                    if (fPlugin.getParameterSymbol(i) != key)
                        continue;

                    fvalue = std::atof(value);
                    fPlugin.setParameterValue(i, fvalue);
# if DISTRHO_PLUGIN_HAS_UI
                    if (fVstUI != nullptr)
                        setParameterValueFromPlugin(i, fvalue);
# endif
                    break;
                }

                // get next key
                size = std::strlen(value) + 1;
                key = value + size;
                bytesRead += size;
            }
        }

        return 1;*/
    }

    case effProcessEvents:
        /*if (!fPlugin.isActive())
        {
            // host has not activated the plugin yet, nasty!
            vst_dispatcher(effMainsChanged, 0, 1, nullptr, 0.0f);
        }

        if (const VstEvents* const events = (const VstEvents*)ptr)
        {
            if (events->numEvents == 0)
                break;

            for (int i = 0, count = events->numEvents; i < count; ++i)
            {
                const VstMidiEvent* const vstMidiEvent((const VstMidiEvent*)events->events[i]);

                if (vstMidiEvent == nullptr)
                    break;
                if (vstMidiEvent->type != kVstMidiType)
                    continue;
                if (fMidiEventCount >= kMaxMidiEvents)
                    break;

                MidiEvent& midiEvent(fMidiEvents[fMidiEventCount++]);
                midiEvent.frame = vstMidiEvent->deltaFrames;
                midiEvent.size = 3;
                std::memcpy(midiEvent.data, vstMidiEvent->midiData, sizeof(uint8_t) * 3);
            }
        }*/
        break;

    case effCanBeAutomated:
        /* if (index < static_cast<int32_t>(fPlugin.getParameterCount()))
       {
             const uint32_t hints(fPlugin.getParameterHints(index));

             // must be automable, and not output
             if ((hints & kParameterIsAutomable) != 0 && (hints & kParameterIsOutput) == 0)
                 return 1;
         }*/
        break;

    case effCanDo:
        /* if (const char* const canDo = (const char*)ptr)
         {
 #if DISTRHO_OS_MAC && DISTRHO_PLUGIN_HAS_UI
             if (std::strcmp(canDo, "hasCockosViewAsConfig") == 0)
             {
                 fUsingNsView = true;
                 return 0xbeef0000;
             }
 #endif
             if (std::strcmp(canDo, "receiveVstEvents") == 0 ||
                 std::strcmp(canDo, "receiveVstMidiEvent") == 0)
 #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
                 return 1;
 #else
                 return -1;
 #endif
             if (std::strcmp(canDo, "sendVstEvents") == 0 ||
                 std::strcmp(canDo, "sendVstMidiEvent") == 0)
 #if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
                 return 1;
 #else
                 return -1;
 #endif
             if (std::strcmp(canDo, "receiveVstTimeInfo") == 0)
 #if DISTRHO_PLUGIN_WANT_TIMEPOS
                 return 1;
 #else
                 return -1;
 #endif
         }*/
        break;

        //case effStartProcess:
        //case effStopProcess:
        // unused
        //    break;
    };
    // handle advanced opcodes
    // if (validPlugin)
    //     return pluginPtr->vst_dispatcher(opcode, index, value, ptr, opt);

    return 0;
}

static intptr_t vst_dispatcher(AEffect* effect, int32_t opcode, int32_t index, intptr_t value, void* ptr, float opt)
{
    auto data = static_cast<VST2ImplementationData*>(effect->user);
    switch (opcode) {
    case effOpen: ///< no arguments  @see AudioEffect::open
        data->plug->init();
        if (data->plug->getFeatureComponent()->supportsFeature(Feature::MidiInput)&& data->audioMaster)
            data->audioMaster(effect, audioMasterWantMidi, 0, 1, 0, 0); //legacy handling
        break;
    case effClose: ///< no arguments  @see AudioEffect::open
        data->plug->deinit();
        break;
    case effSetProgram: ///< [value]: new program number  @see AudioEffect::setProgram
        break;
    case effGetProgram: ///< [return value]: current program number  @see AudioEffect::getProgram
        break;
    case effSetProgramName: ///< [ptr]: char* with new program name, limited to #kVstMaxProgNameLen  @see AudioEffect::setProgramName
        break;
    case effGetProgramName: ///< [ptr]: char buffer for current program name, limited to #kVstMaxProgNameLen  @see AudioEffect::getProgramName
        break;
    case effGetParamLabel: ///< [ptr]: char buffer for parameter label, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterLabel
        break;
    case effGetParamDisplay: ///< [ptr]: char buffer for parameter display, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterDisplay
        break;
    case effGetParamName: ///< [ptr]: char buffer for parameter name, limited to #kVstMaxParamStrLen  @see AudioEffect::getParameterName
        break;
    case effSetSampleRate: ///< [opt]: new sample rate for audio processing  @see AudioEffect::setSampleRate
        break;
    case effSetBlockSize: ///< [value]: new maximum block size for audio processing  @see AudioEffect::setBlockSize
        break;
    case effMainsChanged: ///< [value]: 0 means "turn off", 1 means "turn on"  @see AudioEffect::suspend @see AudioEffect::resume
        if (value == 0)
            data->plug->activate();
        else
            data->plug->deactivate();
        break;
    case effEditGetRect: ///< [ptr]: #ERect** receiving pointer to editor size  @see ERect @see AEffEditor::getRect
        break;
    case effEditOpen: ///< [ptr]: system dependent Window pointer, e.g. HWND on Windows  @see AEffEditor::open
        break;
    case effEditClose: ///< no arguments @see AEffEditor::close
        break;
    case effEditIdle: ///< no arguments @see AEffEditor::idle
        break;
    case effGetChunk: ///< [ptr]: void** for chunk data address [index]: 0 for bank, 1 for program  @see AudioEffect::getChunk
        break;
    case effSetChunk: ///< [ptr]: chunk data [value]: byte size [index]: 0 for bank, 1 for program  @see AudioEffect::setChunk
        break;

        /**************************EXTENDING OPCODES*******************************/
    case effProcessEvents: ///< [ptr]: #VstEvents*  @see AudioEffectX::processEvents = effSetChunk + 1        ///< [ptr]: #VstEvents*  @see AudioEffectX::processEventsdes:
        if (data->plug->getFeatureComponent()->supportsFeature(Feature::MidiInput)) {
            auto events = (VstEvents*)ptr;
            if (events->numEvents == 0)
                break;
            for (int i = 0, count = events->numEvents; i < count; ++i)
            {
                auto midiEvent = (const VstMidiEvent*)events->events[i];
                if (midiEvent != nullptr && midiEvent->type == kVstMidiType) {
                    auto midiPort = getPortAt<IMidiPort>(data->plug, 0, PortDirection::Input);
                    if (midiPort != nullptr)
                        midiPort->feed(MidiMessage{ static_cast<uint8_t>(midiEvent->midiData[0]),static_cast<uint8_t>(midiEvent->midiData[1]),static_cast<uint8_t>(midiEvent->midiData[2]) });
                }
            }
        }   
        break;
    case effCanBeAutomated: ///< [index]: parameter index [return value]: 1=true, 0=false  @see AudioEffectX::canParameterBeAutomated
        break;
    case effString2Parameter: ///< [index]: parameter index [ptr]: parameter string [return value]: true for success  @see AudioEffectX::string2parameter
        break;
    case effGetProgramNameIndexed: ///< [index]: program index [ptr]: buffer for program name, limited to #kVstMaxProgNameLen [return value]: true for success  @see AudioEffectX::getProgramNameIndexed
        break;
    case effGetInputProperties: ///< [index]: input index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getInputProperties
        break;
    case effGetOutputProperties: ///< [index]: output index [ptr]: #VstPinProperties* [return value]: 1 if supported  @see AudioEffectX::getOutputProperties
        break;
    case effGetPlugCategory: ///< [return value]: category  @see VstPlugCategory @see AudioEffectX::getPlugCategory
        break;
    case effOfflineNotify: ///< [ptr]: #VstAudioFile array [value]: count [index]: start flag  @see AudioEffectX::offlineNotify
        break;
    case effOfflinePrepare: ///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlinePrepare
        break;
    case effOfflineRun: ///< [ptr]: #VstOfflineTask array [value]: count  @see AudioEffectX::offlineRun
        break;

    case effProcessVarIo: ///< [ptr]: #VstVariableIo*  @see AudioEffectX::processVariableIo
        break;
    case effSetSpeakerArrangement: ///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::setSpeakerArrangement
        if (getNumberOfPorts<IAudioPort>(data->plug, PortDirection::Input) == 0 || getNumberOfPorts<IAudioPort>(data->plug, PortDirection::Output) == 0)
            return 0;
        return ((VstSpeakerArrangement*)value)->numChannels == getPortAt<IAudioPort>(data->plug, 0, PortDirection::Input )->size() &&
               ((VstSpeakerArrangement*)ptr  )->numChannels == getPortAt<IAudioPort>(data->plug, 0, PortDirection::Output)->size();
        break;
    case effSetBypass: ///< [value]: 1 = bypass, 0 = no bypass  @see AudioEffectX::setBypass
        break;
    case effGetEffectName: ///< [ptr]: buffer for effect name, limited to #kVstMaxEffectNameLen  @see AudioEffectX::getEffectName
        break;
    case effGetVendorString: ///< [ptr]: buffer for effect vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getVendorString
        strncpy(static_cast<char*>(ptr), data->plug->getInfoComponent()->getCreatorName().data(), std::min<size_t>(data->plug->getInfoComponent()->getCreatorName().size(), kVstMaxVendorStrLen));
        return true;
    case effGetProductString: ///< [ptr]: buffer for effect vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getProductString
        strncpy(static_cast<char*>(ptr), data->plug->getInfoComponent()->getPluginName().data(), std::min<size_t>(data->plug->getInfoComponent()->getPluginName().size(), kVstMaxProductStrLen));
        return true;
    case effGetVendorVersion: ///< [return value]: vendor-specific version  @see AudioEffectX::getVendorVersion
        break;
    case effVendorSpecific: ///< no definition, vendor specific handling  @see AudioEffectX::vendorSpecific
        break;
    case effCanDo: ///< [ptr]: "can do" string [return value]: 0: "don't know" -1: "no" 1: "yes"  @see AudioEffectX::canDo//strcmp((const char*)ptr, "dididing");
    {
        static const auto boolToCando = [](bool val) {return val ? 1 : -1; };
        auto canDo = static_cast<const char*>(ptr);
        if (std::strcmp(canDo, "receiveVstEvents") == 0///< plug-in can receive MIDI events from Host
            || std::strcmp(canDo, "receiveVstMidiEvent") == 0)///< plug-in can receive MIDI events from Host 
            return boolToCando(data->plug->getFeatureComponent()->supportsFeature(Feature::MidiInput));
        else if (std::strcmp(canDo, "sendVstEvents") == 0  ///< plug-in will send Vst events to Host
            || std::strcmp(canDo, "sendVstMidiEvent") == 0)///< plug-in will send MIDI events to Host
            return boolToCando(data->plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput));
        else if (std::strcmp(canDo, "receiveVstTimeInfo") == 0)  ///< plug-in can receive Time info from Host 
            return 0;
        else if (std::strcmp(canDo, "offline") == 0)   ///< plug-in supports offline functions (#offlineNotify, #offlinePrepare, #offlineRun)
            return -1;
        else if (std::strcmp(canDo, "midiProgramNames") == 0)  ///< plug-in supports function #getMidiProgramName ()
            return -1;
        else if (std::strcmp(canDo, "bypass") == 0)  ///< plug-in supports function #setBypass ()
            return -1;
        return 0; // return that you absolutly dont know, how to handle the requested feature.
    }
    case effGetTailSize: ///< [return value]: tail size (for example the reverb time of a reverb plug-in); 0 is default (return 1 for 'no tail')
        return 1;
    case effGetParameterProperties: ///< [index]: parameter index [ptr]: #VstParameterProperties* [return value]: 1 if supported  @see AudioEffectX::getParameterProperties
        break;
    case effGetVstVersion: ///< [return value]: VST version  @see AudioEffectX::getVstVersion
        return kVstVersion;

        //#if VST_2_1_EXTENSIONS
    case effEditKeyDown: ///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyDown
        break;
    case effEditKeyUp: ///< [index]: ASCII character [value]: virtual key [opt]: modifiers [return value]: 1 if key used  @see AEffEditor::onKeyUp
        break;
    case effSetEditKnobMode: ///< [value]: knob mode 0: circular, 1: circular relativ, 2: linear (CKnobMode in VSTGUI)  @see AEffEditor::setKnobMode
        break;

    case effGetMidiProgramName: ///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: number of used programs, 0 if unsupported  @see AudioEffectX::getMidiProgramName
        break;
    case effGetCurrentMidiProgram: ///< [index]: MIDI channel [ptr]: #MidiProgramName* [return value]: index of current program  @see AudioEffectX::getCurrentMidiProgram
        break;
    case effGetMidiProgramCategory: ///< [index]: MIDI channel [ptr]: #MidiProgramCategory* [return value]: number of used categories, 0 if unsupported  @see AudioEffectX::getMidiProgramCategory
        break;
    case effHasMidiProgramsChanged: ///< [index]: MIDI channel [return value]: 1 if the #MidiProgramName(s) or #MidiKeyName(s) have changed  @see AudioEffectX::hasMidiProgramsChanged
        break;
    case effGetMidiKeyName: ///< [index]: MIDI channel [ptr]: #MidiKeyName* [return value]: true if supported, false otherwise  @see AudioEffectX::getMidiKeyName
        break;

    case effBeginSetProgram: ///< no arguments  @see AudioEffectX::beginSetProgram
        break;
    case effEndSetProgram: ///< no arguments  @see AudioEffectX::endSetProgram
        break;
        //#endif // VST_2_1_EXTENSIONS

        //#if VST_2_3_EXTENSIONS
    case effGetSpeakerArrangement: ///< [value]: input #VstSpeakerArrangement* [ptr]: output #VstSpeakerArrangement*  @see AudioEffectX::getSpeakerArrangement
        // IMplement changing of Inputconfiguration here.
        return false;
        break;
    case effShellGetNextPlugin: ///< [ptr]: buffer for plug-in name, limited to #kVstMaxProductStrLen [return value]: next plugin's uniqueID  @see AudioEffectX::getNextShellPlugin
        break;

    case effStartProcess: ///< no arguments  @see AudioEffectX::startProcess
        break;
    case effStopProcess: ///< no arguments  @see AudioEffectX::stopProcess
        break;
    case effSetTotalSampleToProcess: ///< [value]: number of samples to process, offline only!  @see AudioEffectX::setTotalSampleToProcess
        break;
    case effSetPanLaw: ///< [value]: pan law [opt]: gain  @see VstPanLawType @see AudioEffectX::setPanLaw
        break;

    case effBeginLoadBank: ///< [ptr]: #VstPatchChunkInfo* [return value]: -1: bank can't be loaded, 1: bank can be loaded, 0: unsupported  @see AudioEffectX::beginLoadBank
        break;
    case effBeginLoadProgram: ///< [ptr]: #VstPatchChunkInfo* [return value]: -1: prog can't be loaded, 1: prog can be loaded, 0: unsupported  @see AudioEffectX::beginLoadProgram
        break;
        //#endif // VST_2_3_EXTENSIONS

        //#if VST_2_4_EXTENSIONS
    case effSetProcessPrecision: ///< [value]: @see VstProcessPrecision  @see AudioEffectX::setProcessPrecision
        break;
    case effGetNumMidiInputChannels: ///< [return value]: number of used MIDI input channels (1-15)  @see AudioEffectX::getNumMidiInputChannels
        break;
    case effGetNumMidiOutputChannels: ///< [return value]: number of used MIDI output channels (1-15)  @see AudioEffectX::getNumMidiOutputChannels
        break;
        //#endif // VST_2_4_EXTENSIONS
    }
    return 0;
}

void writeMidiOutput(AEffect* effect) {
    auto data = static_cast<VST2ImplementationData*>(effect->user);
    if (data->plug->getFeatureComponent()->supportsFeature(Feature::MidiOutput)) {
        iteratePorts<IMidiPort>(data->plug, PortDirection::Output, [&data, &effect](IMidiPort* p, size_t ind) {
            while (!p->empty()) {
                auto midiMsg = p->get();
                VstMidiEvent vstMidiEvent{ kVstMidiType,sizeof(VstMidiEvent),0,0,0,0,{static_cast<char>(midiMsg[0]),static_cast<char>(midiMsg[1]),static_cast<char>(midiMsg[2]),0 },0,0,0,0 };
                VstEvents vstEvents{ 1,0,(VstEvent*)&vstMidiEvent };
                data->audioMaster(effect, audioMasterProcessEvents, static_cast<int32_t>(ind), 0, &vstEvents, 0);
            }
            return false;
            });
    }
}

audioMasterCallback globalAudioMaster;
#include <iostream>
// -----------------------------------------------------------------------
extern "C" {
const AEffect* VSTPluginMain(audioMasterCallback audioMaster)
{
    globalAudioMaster = audioMaster;

    //PluginController plugin = GlobalData().getPlugin(0);
    // old version
    if (audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0.0f) == 0)
        return nullptr;
    // first internal init
    //  vst_dispatcherCallback(nullptr, -1729, 0xdead, 0xf00d, &plugin, 0.0f);
    AEffect* const effect(new AEffect);
    VST2ImplementationData* const data(new  VST2ImplementationData);
    data->audioMaster = audioMaster;
    data->plug = GlobalData().getPlugin(0).get();
    effect->user = data;
    // std::memset(effect, 0, sizeof(AEffect));
    effect->magic = kEffectMagic;
    effect->dispatcher = vst_dispatcher;

    /******************INFORMATION*****************/
    effect->uniqueID = 0;
    effect->version = 0;
    effect->numInputs = static_cast<int>( getNumberOfPorts<IAudioPort>(data->plug,PortDirection::Input));
    effect->numOutputs = static_cast<int>(getNumberOfPorts<IAudioPort>(data->plug, PortDirection::Output));
    effect->numParams = 0;
    effect->numPrograms = 0;
  
    //
    if (audioMaster) {

      /*  //OPCodes
        audioMaster(nullptr, audioMasterAutomate, 0, 0, nullptr, 0.0f);///< [index]: parameter index [opt]: parameter value  @see AudioEffect::setParameterAutomated
        audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0.0f);///< [return value]: Host VST version (for example 2400 for VST 2.4) @see AudioEffect::getMasterVersion
        audioMaster(nullptr, audioMasterCurrentId, 0, 0, nullptr, 0.0f);///< [return value]: current unique identifier on shell plug-in  @see AudioEffect::getCurrentUniqueId
        audioMaster(nullptr, audioMasterIdle, 0, 0, nullptr, 0.0f);	///< no arguments  @see AudioEffect::masterIdle
        audioMaster(nullptr, audioMasterPinConnected, 0, 0, nullptr, 0.0f);///< [return value]: 0=true, 1=false [index]: pin index [value]: 0=input, 1=output  @see AudioEffect::isInputConnected @see AudioEffect::isOutputConnected
                                                                 

        //OPCodes 2.X
        audioMaster(nullptr, audioMasterGetTime, 0, 0, nullptr, 0);///< [return value]: #VstTimeInfo* or null if not supported [value]: request mask  @see VstTimeInfoFlags @see AudioEffectX::getTimeInfo
        audioMaster(nullptr, audioMasterProcessEvents, 0, 0, nullptr, 0);///< [ptr]: pointer to #VstEvents  @see VstEvents @see AudioEffectX::sendVstEventsToHost
        audioMaster(nullptr, audioMasterIOChanged, 0, 0, nullptr, 0);///< [return value]: 1 if supported  @see AudioEffectX::ioChanged
        audioMaster(nullptr, audioMasterSizeWindow, 0, 0, nullptr, 0);	///< [index]: new width [value]: new height [return value]: 1 if supported  @see AudioEffectX::sizeWindow
        audioMaster(nullptr, audioMasterGetSampleRate, 0, 0, nullptr, 0);///< [return value]: current sample rate  @see AudioEffectX::updateSampleRate
        audioMaster(nullptr, audioMasterGetBlockSize, 0, 0, nullptr, 0);	///< [return value]: current block size  @see AudioEffectX::updateBlockSize
        audioMaster(nullptr, audioMasterGetInputLatency, 0, 0, nullptr, 0);///< [return value]: input latency in audio samples  @see AudioEffectX::getInputLatency
        audioMaster(nullptr, audioMasterGetOutputLatency, 0, 0, nullptr, 0);///< [return value]: output latency in audio samples  @see AudioEffectX::getOutputLatency
        audioMaster(nullptr, audioMasterGetCurrentProcessLevel, 0, 0, nullptr, 0);///< [return value]: current process level  @see VstProcessLevels
        audioMaster(nullptr, audioMasterGetAutomationState, 0, 0, nullptr, 0);	///< [return value]: current automation state  @see VstAutomationStates
        audioMaster(nullptr, audioMasterOfflineStart, 0, 0, nullptr, 0);///< [index]: numNewAudioFiles [value]: numAudioFiles [ptr]: #VstAudioFile*  @see AudioEffectX::offlineStart
        audioMaster(nullptr, audioMasterOfflineRead, 0, 0, nullptr, 0);///< [index]: bool readSource [value]: #VstOfflineOption* @see VstOfflineOption [ptr]: #VstOfflineTask*  @see VstOfflineTask @see AudioEffectX::offlineRead
        audioMaster(nullptr, audioMasterOfflineWrite, 0, 0, nullptr, 0);///< @see audioMasterOfflineRead @see AudioEffectX::offlineRead
        audioMaster(nullptr, audioMasterOfflineGetCurrentPass, 0, 0, nullptr, 0);///< @see AudioEffectX::offlineGetCurrentPass
        audioMaster(nullptr, audioMasterOfflineGetCurrentMetaPass, 0, 0, nullptr, 0);///< @see AudioEffectX::offlineGetCurrentMetaPass
        audioMaster(nullptr, audioMasterGetVendorString, 0, 0, nullptr, 0);///< [ptr]: char buffer for vendor string, limited to #kVstMaxVendorStrLen  @see AudioEffectX::getHostVendorString
        audioMaster(nullptr, audioMasterGetProductString, 0, 0, nullptr, 0);///< [ptr]: char buffer for vendor string, limited to #kVstMaxProductStrLen  @see AudioEffectX::getHostProductString
        audioMaster(nullptr, audioMasterGetVendorVersion, 0, 0, nullptr, 0);///< [return value]: vendor-specific version  @see AudioEffectX::getHostVendorVersion
        audioMaster(nullptr, audioMasterVendorSpecific, 0, 0, nullptr, 0);	///< no definition, vendor specific handling  @see AudioEffectX::hostVendorSpecific
        audioMaster(nullptr, audioMasterCanDo, 0, 0, nullptr, 0);///< [ptr]: "can do" string [return value]: 1 for supported
        audioMaster(nullptr, audioMasterGetLanguage, 0, 0, nullptr, 0);	///< [return value]: language code  @see VstHostLanguage
        audioMaster(nullptr, audioMasterGetDirectory, 0, 0, nullptr, 0);///< [return value]: FSSpec on MAC, else char*  @see AudioEffectX::getDirectory
        audioMaster(nullptr, audioMasterUpdateDisplay, 0, 0, nullptr, 0);	///< no arguments	
        audioMaster(nullptr, audioMasterBeginEdit, 0, 0, nullptr, 0);   ///< [index]: parameter index  @see AudioEffectX::beginEdit
        audioMaster(nullptr, audioMasterEndEdit, 0, 0, nullptr, 0);  ///< [index]: parameter index  @see AudioEffectX::endEdit
        audioMaster(nullptr, audioMasterOpenFileSelector, 0, 0, nullptr, 0);///< [ptr]: VstFileSelect* [return value]: 1 if supported  @see AudioEffectX::openFileSelector
        audioMaster(nullptr, audioMasterCloseFileSelector, 0, 0, nullptr, 0);///< [ptr]: VstFileSelect*  @see AudioEffectX::closeFileSelector
        */
    
        //Deprecated:
        /* audioMaster(nullptr, audioMasterWantMidi, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterSetTime, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterTempoAt, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetNumAutomatableParameters, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetParameterQuantization, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterNeedIdle, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetPreviousPlug, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetNextPlug, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterWillReplaceOrAccumulate, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterSetOutputSampleRate, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetOutputSpeakerArrangement, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterSetIcon, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterOpenWindow, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterCloseWindow, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterEditFile, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetChunkFile, 0, 0, nullptr, 0);
         audioMaster(nullptr, audioMasterGetInputSpeakerArrangement, 0, 0, nullptr, 0);*/
    }
   
    effect->processReplacing = [](AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
        auto data = static_cast<VST2ImplementationData*>(effect->user);
        int inputIndex = 0;
        int outputIndex = 0;
        iteratePorts<IAudioPort>(data->plug, [sampleFrames,&inputs,&inputIndex,&outputs,&outputIndex](IAudioPort* p, size_t) {
            p->setSampleSize(static_cast<size_t>(sampleFrames));
            for (size_t i = 0; i < p->size(); i++) {
                if (p->getDirection() == PortDirection::Input) {
                    p->at(i)->feed(inputs[inputIndex]);
                    inputIndex++;
                }
                else {
                    p->at(i)->feed(outputs[outputIndex]);
                    outputIndex++;
                }
            }
            return false;
            });
        
        data->plug->processAudio();
        writeMidiOutput(effect);
    };

    effect->processDoubleReplacing = [](AEffect* effect, double** inputs, double** outputs, int32_t sampleFrames) {
    /*    auto data = static_cast<VST2ImplementationData*>(effect->user);
        int inputIndex = 0;
        int outputIndex = 0;
        iteratePorts<IAudioPort>(data->plug, [sampleFrames, inputs, &inputIndex, outputs, &outputIndex](IAudioPort* p, size_t) {
            p->setSampleSize(static_cast<size_t>(sampleFrames));
            for (size_t i = 0; i < p->size(); i++) {
                if (p->getDirection() == PortDirection::Input) {
                    p->at(i)->feed( nullptr, inputs[inputIndex]);
                    inputIndex++;
                }
                else {
                    p->at(i)->feed( nullptr, outputs[outputIndex]);
                    outputIndex++;
                }
            }
            return false;
            });

        data->plug->processAudio();
        writeMidiOutput(effect);*/
    };
    effect->flags |= effFlagsCanReplacing | effFlagsCanDoubleReplacing;
    if (data->plug->getFeatureComponent()->supportsFeature(Feature::GUISupport))
        effect->flags |= effFlagsHasEditor;


    effect->getParameter = [](AEffect* effect, int32_t index) -> float {
        auto data = static_cast<VST2ImplementationData*>(effect->user);
        return 0; };
    effect->setParameter = [](AEffect* effect, int32_t index, float value) {
        auto data = static_cast<VST2ImplementationData*>(effect->user);
    };

    // pointers

    // done
    // effect->object = obj;

    return effect;
}
}
