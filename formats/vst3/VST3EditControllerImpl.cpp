#include "VST3EditControllerImpl.hpp"

VST3EditControllerImpl::VST3EditControllerImpl() : __funknownRefCount(1){
}

tresult PLUGIN_API VST3EditControllerImpl::queryInterface(const TUID _iid, void** obj)
{
    //QUERY_INTERFACE(_iid, obj, IEditController2::iid, IEditController2)
    QUERY_INTERFACE(_iid, obj, IEditController::iid, IEditController)
    QUERY_INTERFACE(_iid, obj, IPluginBase::iid, IPluginBase)
    QUERY_INTERFACE(_iid, obj, FUnknown::iid, FUnknown)
        * obj = nullptr;
    return kNoInterface;
}

uint32 PLUGIN_API VST3EditControllerImpl::addRef()
{
    return ::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, 1);
}

uint32 PLUGIN_API VST3EditControllerImpl::release()
{
    if (::Steinberg::FUnknownPrivate::atomicAdd(__funknownRefCount, -1) == 0)
    {
        delete this;
        return 0;
    }
    return __funknownRefCount;
}

tresult PLUGIN_API VST3EditControllerImpl::initialize(FUnknown* )
{
    return kResultOk;
}

tresult PLUGIN_API VST3EditControllerImpl::terminate()
{
    return kResultOk;
}

tresult PLUGIN_API VST3EditControllerImpl::setComponentState(IBStream* )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::setState(IBStream* )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::getState(IBStream* )
{
    return kNotImplemented;
}

int32 PLUGIN_API VST3EditControllerImpl::getParameterCount()
{
    return 0;
}

tresult PLUGIN_API VST3EditControllerImpl::getParameterInfo(int32 , ParameterInfo& )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::getParamStringByValue(ParamID , ParamValue , String128 )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::getParamValueByString(ParamID , TChar* , ParamValue& )
{
    return kNotImplemented;
}
ParamValue PLUGIN_API VST3EditControllerImpl::normalizedParamToPlain(ParamID , ParamValue )
{
    return 0;
}

ParamValue PLUGIN_API VST3EditControllerImpl::plainParamToNormalized(ParamID , ParamValue )
{
    return kNotImplemented;
}

ParamValue PLUGIN_API VST3EditControllerImpl::getParamNormalized(ParamID )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::setParamNormalized(ParamID , ParamValue )
{
    return kNotImplemented;
}

tresult PLUGIN_API VST3EditControllerImpl::setComponentHandler(IComponentHandler* )
{
    return kNotImplemented;
}

IPlugView* PLUGIN_API VST3EditControllerImpl::createView(FIDString )
{
    return nullptr;
}
/*
tresult PLUGIN_API VST3EditControllerImpl::setKnobMode(KnobMode mode)
{
    return tresult PLUGIN_API();
}

tresult PLUGIN_API VST3EditControllerImpl::openHelp(TBool onlyCheck)
{
    return tresult PLUGIN_API();
}

tresult PLUGIN_API VST3EditControllerImpl::openAboutBox(TBool onlyCheck)
{
    return tresult PLUGIN_API();
}
*/

const ::Steinberg::FUID VST3EditControllerImpl::cid(INLINE_UID(0x00000115, 0x00241011, 0x00011000, 0x00000110));