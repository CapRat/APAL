#ifndef VST3_AUDIO_PROCESSOR_IMPL_HPP
#define VST3_AUDIO_PROCESSOR_IMPL_HPP
#include <vst/ivstaudioprocessor.h>
#include "Component.hpp"
using namespace Steinberg;
using namespace Vst;

class VST3AudioProccessorImpl : public Component, public IAudioProcessor
{
public:
	static const FUID cid;

	VST3AudioProccessorImpl();
	virtual ~VST3AudioProccessorImpl();
	// Geerbt über Component
	virtual tresult PLUGIN_API queryInterface(const TUID _iid, void** obj) override;


	inline virtual uint32 PLUGIN_API addRef() override;

	inline virtual uint32 PLUGIN_API release() override;


	// Geerbt über IAudioProcessor

	/** Try to set (from host) a predefined arrangement for inputs and outputs.
		The host should always deliver the same number of input and output buses than the Plug-in needs
		(see \ref IComponent::getBusCount).
		The Plug-in returns kResultFalse if wanted arrangements are not supported.
		If the Plug-in accepts these arrangements, it should modify its buses to match the new arrangements
		(asked by the host with IComponent::getInfo () or IAudioProcessor::getBusArrangement ()) and then return kResultTrue.
		If the Plug-in does not accept these arrangements, but can adapt its current arrangements (according to the wanted ones),
		it should modify its buses arrangements and return kResultFalse. */
	virtual tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) override;

	/** Gets the bus arrangement for a given direction (input/output) and index.
		Note: IComponent::getInfo () and IAudioProcessor::getBusArrangement () should be always return the same
		information about the buses arrangements. */
	virtual tresult PLUGIN_API getBusArrangement(BusDirection dir, int32 index, SpeakerArrangement& arr) override;

	/** Asks if a given sample size is supported see \ref SymbolicSampleSizes. */
	virtual tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize) override;

	/** Gets the current Latency in samples.
		The returned value defines the group delay or the latency of the Plug-in. For example, if the Plug-in internally needs
		to look in advance (like compressors) 512 samples then this Plug-in should report 512 as latency.
		If during the use of the Plug-in this latency change, the Plug-in has to inform the host by
		using IComponentHandler::restartComponent (kLatencyChanged), this could lead to audio playback interruption
		because the host has to recompute its internal mixer delay compensation.
		Note that for player live recording this latency should be zero or small. */
	virtual uint32 PLUGIN_API getLatencySamples() override;

	/** Called in disable state (setActive not called with true) before setProcessing is called and processing will begin. */
	virtual tresult PLUGIN_API setupProcessing(ProcessSetup& newSetup) override;

	/** Informs the Plug-in about the processing state. This will be called before any process calls
	   start with true and after with false.
	   Note that setProcessing (false) may be called after setProcessing (true) without any process
	   calls. In this call the Plug-in should do only light operation (no memory allocation or big
	   setup reconfiguration), this could be used to reset some buffers (like Delay line or Reverb).
	   The host has to be sure that it is called only when the Plug-in is enable (setActive was
	   called) */
	virtual tresult PLUGIN_API setProcessing(TBool state) override;

	/** The Process call, where all information (parameter changes, event, audio buffer) are passed. */
	virtual tresult PLUGIN_API process(ProcessData& data) override;

	/** Gets tail size in samples. For example, if the Plug-in is a Reverb Plug-in and it knows that
		the maximum length of the Reverb is 2sec, then it has to return in getTailSamples()
		(in VST2 it was getGetTailSize ()): 2*sampleRate.
		This information could be used by host for offline processing, process optimization and
		downmix (avoiding signal cut (clicks)).
		It should return:
		 - kNoTail when no tail
		 - x * sampleRate when x Sec tail.
		 - kInfiniteTail when infinite tail. */
	virtual uint32 PLUGIN_API getTailSamples() override;

protected:
	ProcessSetup processSetup;
	
};


#endif //! VST3_AUDIO_PROCESSOR_IMPL_HPP