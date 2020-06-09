/**
 * File to check, which VST2 Version should be used. It should make sure, that everything is defined. If USE_VST2_SDK variable is set, the official sdk2 headers are used.
 */

#ifndef VST_DEF_HPP
#define VST_DEF_HPP

#ifdef USE_VST2_SDK
// Use Official VST2 Sdk
#include "aeffectx.h"
//# include "vst/aeffectx.h"
#else
#define VESTIGE_HEADER
#include "vestige.h"
// Use Vestigeheader, with some reverse engeneered values.
#define effFlagsProgramChunks (1 << 5)
#define effFlagsCanDoubleReplacing  1 << 12

#define kVstVersion 2400

#define effSetProgramName 4
#define effGetParamLabel 6
#define effGetParamDisplay 7

#define effGetChunk 23
#define effSetChunk 24
#define effCanBeAutomated 26
#define effString2Parameter 27
#define effGetProgramNameIndexed 29
#define effGetInputProperties 33
#define effGetOutputProperties 34
#define effGetPlugCategory 35
#define effOfflineNotify 38
#define effOfflinePrepare 39
#define effOfflineRun 40
#define effProcessVarIo 41
#define effSetSpeakerArrangement 42
#define effSetBypass 44  
#define effVendorSpecific 50
#define effGetTailSize 52
#define effEditKeyDown 59
#define effEditKeyUp 60
#define effSetEditKnobMode 61
#define effGetMidiProgramName 62
#define effGetCurrentMidiProgram 63
#define effGetMidiProgramCategory 64
#define effHasMidiProgramsChanged 65
#define effGetMidiKeyName 66
#define effGetSpeakerArrangement 69
#define effSetTotalSampleToProcess 73
#define effSetPanLaw 74
#define effBeginLoadBank 75
#define effBeginLoadProgram 76
#define effSetProcessPrecision 77
#define effGetNumMidiInputChannels 78
#define effGetNumMidiOutputChannels 79

// used for vestige and sdk compatibility
enum VstStringConstants
{
	kVstMaxProgNameLen = VestigeMaxCategLabelLen,
	kVstMaxParamStrLen = VestigeMaxShortLabelLen,
	kVstMaxVendorStrLen = VestigeMaxNameLen,	
	kVstMaxProductStrLen = VestigeMaxLabelLen,
	kVstMaxEffectNameLen = VestigeMaxCategLabelLen + VestigeMaxShortLabelLen
};

struct ERect {
	int16_t top, left, bottom, right;
};

#endif 


#endif //! VST_DEF_HPP