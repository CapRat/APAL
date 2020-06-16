#include <core/lv2.h>
#include <midi/midi.h>
#include <interfaces/IPlugin.hpp>
#include "GlobalData.hpp"
#include <tools/PluginUtils.hpp>
#include <vector>
#include <unordered_map>
using namespace XPlug;

struct LV2HandleDataType {
    IPlugin* plug;
    const LV2_Descriptor* lv2Desc;
};



/**
   The `lv2_descriptor()` function is the entry point to the plugin library.  The
   host will load symbols the library and call this function repeatedly with increasing
   indices to find all the plugins defined in the library.  The index is not an
   indentifier, the URI of the returned descriptor is used to determine the
   identify of the plugin.

   This method is in the ``discovery'' threading class, so no other functions
   or methods in this plugin library will be called concurrently with it.
*/
static std::unordered_map<std::string, uint32_t>  URI_INDEX_MAP;
extern "C" {
    static int currentLV2Index = 0;
    const LV2_Descriptor* lv2_descriptor(uint32_t index)
    {
        
        if (index < 0 || index >= GlobalData().getNumberOfRegisteredPlugins())
            return NULL;
        PluginPtr plug = GlobalData().getPlugin(index);

        auto desc = new LV2_Descriptor();

        desc->URI=plug->getPluginInfo()->url.c_str();
        URI_INDEX_MAP[plug->getPluginInfo()->url]= index;

        //LV2_Descriptor desc = lv2DescriptorArray.at(lv2DescriptorArray.size() - 1);
        desc->activate = [](LV2_Handle instance) {
            auto data = static_cast<LV2HandleDataType*>(instance);
            data->plug->activate();
        };
        desc->deactivate = [](LV2_Handle instance) {
            auto data = static_cast<LV2HandleDataType*>(instance); 
            data->plug->deactivate();
        };
        desc->connect_port = [](LV2_Handle instance, uint32_t IPort, void* DataLocation) {
            auto data = static_cast<LV2HandleDataType*>(instance);
            if (DataLocation != nullptr) {
        
                getChannelFromIndex(data->plug, IPort)->feed({ (float*)DataLocation,(double*)DataLocation });
            }
        };
    
        desc->instantiate = [](const LV2_Descriptor* descriptor, double SampleRate, const char* bundlePath, const LV2_Feature* const* features)->LV2_Handle {
            auto index = URI_INDEX_MAP[std::string(descriptor->URI)];
            GlobalData().getPlugin(index)->init();
            return new LV2HandleDataType{GlobalData().getPlugin(index).get(),descriptor };
        };

        desc->cleanup = [](LV2_Handle instance) {
            auto data =static_cast<LV2HandleDataType *>( instance);
            URI_INDEX_MAP.erase(data->plug->getPluginInfo()->url);
            data->plug->deinit();
            delete data->lv2Desc;
            delete data;
        };

        desc->run = [](LV2_Handle instance, uint32_t SampleCount) {
            auto data = static_cast<LV2HandleDataType*>(instance); 
            //TODO not nice. Maybe write a function, which does this, but is RT Capable (non mem allocation and blocking ist allowed.
            iteratePorts(data->plug, [SampleCount](IPort* p, size_t index) {
                if (p->getType() == PortType::Audio) {
                    dynamic_cast<IAudioPort*>(p)->setSampleSize(SampleCount);
                }
                return false;
                });
            data->plug->processAudio();
        };

        desc->extension_data = [](const char* uri)-> const void* {
            return NULL;
        };

        return desc;
    }

    const LV2_Lib_Descriptor* lv2_lib_descriptor(const char* bundle_path, const LV2_Feature* const* features)
    {
        return nullptr;
    }
}