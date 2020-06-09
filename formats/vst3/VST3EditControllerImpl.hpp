#ifndef VST3_EDIT_CONTROLLER_IMPL_HPP
#define VST3_EDIT_CONTROLLER_IMPL_HPP
#include "ivsteditcontroller.h"
using namespace Steinberg;
using namespace Vst;
class VST3EditControllerImpl : public Steinberg::Vst::IEditController{
public:
	static const FUID cid;
	VST3EditControllerImpl();
	// Geerbt über IEditController
	virtual tresult PLUGIN_API queryInterface(const TUID _iid, void** obj) override;
	virtual uint32 PLUGIN_API addRef() override;
	virtual uint32 PLUGIN_API release() override;
	virtual tresult PLUGIN_API initialize(FUnknown* context) override;
	virtual tresult PLUGIN_API terminate() override;
	virtual tresult PLUGIN_API setComponentState(IBStream* state) override;
	virtual tresult PLUGIN_API setState(IBStream* state) override;
	virtual tresult PLUGIN_API getState(IBStream* state) override;
	virtual int32 PLUGIN_API getParameterCount() override;
	virtual tresult PLUGIN_API getParameterInfo(int32 paramIndex, ParameterInfo& info) override;
	virtual tresult PLUGIN_API getParamStringByValue(ParamID id, ParamValue valueNormalized, String128 string) override;
	virtual tresult PLUGIN_API getParamValueByString(ParamID id, TChar* string, ParamValue& valueNormalized) override;
	virtual ParamValue PLUGIN_API normalizedParamToPlain(ParamID id, ParamValue valueNormalized) override;
	virtual ParamValue PLUGIN_API plainParamToNormalized(ParamID id, ParamValue plainValue) override;
	virtual ParamValue PLUGIN_API getParamNormalized(ParamID id) override;
	virtual tresult PLUGIN_API setParamNormalized(ParamID id, ParamValue value) override;
	virtual tresult PLUGIN_API setComponentHandler(IComponentHandler* handler) override;
	virtual IPlugView* PLUGIN_API createView(FIDString name) override;

	// Geerbt über IEditController2
/*	virtual tresult PLUGIN_API setKnobMode(KnobMode mode) override;
	virtual tresult PLUGIN_API openHelp(TBool onlyCheck) override;
	virtual tresult PLUGIN_API openAboutBox(TBool onlyCheck) override;*/
protected:
	int32 __funknownRefCount;
};

#endif //! VST3_EDIT_CONTROLLER_IMPL_HPP