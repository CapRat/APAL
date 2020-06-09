#define INIT_CLASS_IID
#include "VST3AudioProcessorImpl.hpp"
#include "VST3EditControllerImpl.hpp"
//#include "public.sdk/source/vst/vstcomponent.h"
#include "interfaces/IPlugin.hpp"
#include "GlobalData.hpp"
using namespace Steinberg;
using namespace Vst;


/****************INITIALILSATION*******************/



bool InitModule() { return true; }
bool DeinitModule() { return true; }

extern "C" {
    bool  InitDll() ///< must be called from host right after loading dll
    {
        return InitModule();
    }
    bool  ExitDll()  ///< must be called from host right before unloading dll
    {
        return DeinitModule();
    }

    void* moduleHandle = 0;


    static int counter{ 0 };

    bool ModuleEntry(void* sharedLibraryHandle)
    {
        if (++counter == 1)
        {
            moduleHandle = sharedLibraryHandle;
            return InitModule();
        }
        return true;
    }

    bool ModuleExit(void)
    {
        if (--counter == 0)
        {
            moduleHandle = nullptr;
            return DeinitModule();
        }
        return true;
    }


}

#include "public.sdk/source/main/pluginfactory.h"


//#define stringPluginName "Supercool2"



extern "C" {
    IPluginFactory* PLUGIN_API GetPluginFactory()
    {
        if (!gPluginFactory)
        {
            auto plug = XPlug::GlobalData().getPlugin(0);
        //    plug->getPluginInfo()->creater
            auto inf = plug->getPluginInfo();
            static PFactoryInfo factoryInfo =
            {
                plug->getPluginInfo()->creater.c_str(),
                "http://www.mywebpage.com",
                "mailto:myemail@address.com",
                PFactoryInfo::kNoFlags
            };

            gPluginFactory = new CPluginFactory(factoryInfo);
        
            static PClassInfo2 componentClass(
                VST3AudioProccessorImpl::cid, //cid
                PClassInfo::kManyInstances,//cardinality
                kVstAudioEffectClass, //category
                plug->getPluginInfo()->name.c_str(), //name
                NULL, // class flags
                "Fx|Delay", //subcategory
                0, //vendor
                "1.0.0.0", //version
                kVstVersionString //sdkversion
            );
            gPluginFactory->registerClass(&componentClass, [](void* data)->FUnknown* {return   (IAudioProcessor*)new VST3AudioProccessorImpl(); });
            static std::string controllerName = plug->getPluginInfo()->name + "Controller";
            static PClassInfo2 componentContollerClass(
                VST3EditControllerImpl::cid, //cid
                PClassInfo::kManyInstances,//cardinality
                kVstComponentControllerClass, //category
                controllerName.c_str(), //name
                NULL, // class flags
                "", //subcategory
                0, //vendor
                "1.0.0.0", //version
                kVstVersionString //sdkversion
            );
            gPluginFactory->registerClass(&componentContollerClass, [](void* data)->FUnknown* {return   (IEditController*)new VST3EditControllerImpl(); });
        }
        else
            gPluginFactory->addRef();

        return gPluginFactory;
    }
}

