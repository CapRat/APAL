
#define VESTIGE_HEADER
#ifdef VESTIGE_HEADER
# include "vestige.h"
#define effFlagsProgramChunks (1 << 5)
#define effSetProgramName 4
#define effGetParamLabel 6
#define effGetParamDisplay 7
#define effGetChunk 23
#define effSetChunk 24
#define effCanBeAutomated 26
#define effGetProgramNameIndexed 29
#define effGetPlugCategory 35
#define effEditKeyDown 59
#define effEditKeyUp 60
#define kVstVersion 2400
struct ERect {
    int16_t top, left, bottom, right;
};
#else
# include "vst/aeffectx.h"
#endif

#include <GlobalData.hpp>
#include <IPlugin.hpp>
intptr_t vst_dispatcher(const int32_t opcode, const int32_t index, const intptr_t value, void* const ptr, const float opt)
{
#if DISTRHO_PLUGIN_WANT_STATE
    intptr_t ret = 0;
#endif

    switch (opcode)
    {
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


    case effGetChunk:
    {
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

    case effSetChunk:
    {
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
    }

    return 0;
}

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
    switch (opcode)
    {
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
        /*if (char* const cptr = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(cptr, plugin.getName(), 32);
            return 1;
        }*/
        return 0;

    case effGetVendorString:
        /*if (char* const cptr = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(cptr, plugin.getMaker(), 32);
            return 1;
        }*/
        return 0;

    case effGetProductString:
        /*if (char* const cptr = (char*)ptr)
        {
            DISTRHO_NAMESPACE::strncpy(cptr, plugin.getLabel(), 32);
            return 1;
        }*/
        return 0;

    case effGetVendorVersion:
       // return plugin.getVersion();

    case effGetVstVersion:
        //return kVstVersion;

  
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


    case effGetChunk:
    {
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

    case effSetChunk:
    {
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


#include <iostream>
// -----------------------------------------------------------------------
extern "C" {
    const AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
        //PluginController plugin = GlobalData().getPlugin(0);
        // old version
        if (audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0.0f) == 0)
            return nullptr;

        // first internal init
      //  vst_dispatcherCallback(nullptr, -1729, 0xdead, 0xf00d, &plugin, 0.0f);

        AEffect* const effect(new AEffect);
        // std::memset(effect, 0, sizeof(AEffect));

         // vst fields
        effect->magic = kEffectMagic;
       // effect->uniqueID = //plugin->getUniqueId();
        //effect->version = plugin->getPluginInfo().ver;
        //effect->numParams = plugin->getParameterCount();
        effect->numPrograms = 1;
        //  effect->numInputs = DISTRHO_PLUGIN_NUM_INPUTS;
        //  effect->numOutputs = DISTRHO_PLUGIN_NUM_OUTPUTS;
       
  //  int numParams = 0;
    //bool outputsReached = false;

   /* for (uint32_t i = 0, count = plugin->getParameterCount(); i < count; ++i)
    {
        if (plugin->isParameterInput(i))
        {
            // parameter outputs must be all at the end
      //      DISTRHO_SAFE_ASSERT_BREAK(!outputsReached);
            ++numParams;
            continue;
        }
        outputsReached = true;
    }*/


    // plugin fields
        // effect->numParams = numParams;
         //   effect->numPrograms = 1;
        //  effect->numInputs = DISTRHO_PLUGIN_NUM_INPUTS;
        //  effect->numOutputs = DISTRHO_PLUGIN_NUM_OUTPUTS;

          // plugin flags
        //effect->flags |= effFlagsCanReplacing;
        /*#if DISTRHO_PLUGIN_IS_SYNTH
            effect->flags |= effFlagsIsSynth;
        #endif
        #if DISTRHO_PLUGIN_HAS_UI
            effect->flags |= effFlagsHasEditor;
        #endif
        #if DISTRHO_PLUGIN_WANT_STATE
            effect->flags |= effFlagsProgramChunks;
        #endif*/
     

        effect->numInputs = 1;
        effect->numOutputs = 1;
        effect->numParams = 0;
        effect->numPrograms = 0;
        // static calls
        effect->dispatcher = vst_dispatcherCallback;
        effect->process = [](AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
            // std::cout << "process";
            std::vector<audio_data> inputsD;
            std::vector<audio_data> outputsD;
            audio_data input;
            input.assign(inputs[0][0], inputs[0][sampleFrames]);
            audio_data output;
            input.assign(outputs[0][0], outputs[0][sampleFrames]);
           
          
            GlobalData().getPlugin(0)->processAudio(inputsD, outputsD);
     
           
        };
        effect->getParameter = [](AEffect* effect, int32_t index)->float {return 0; };
        effect->setParameter = [](AEffect* effect, int32_t index, float value) {};
        effect->processReplacing = [](AEffect* effect, float** inputs, float** outputs, int32_t sampleFrames) {
            std::cout << "processReplacing";
            audio_data x;
           // GlobalData().getPlugin(0)->processAudio()
        };

        // pointers

        // done
       // effect->object = obj;

        return effect;
    }
}