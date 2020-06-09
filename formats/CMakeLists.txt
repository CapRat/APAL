if(XPLUG_LADSPA_SUPPORT)
add_subdirectory(ladspa)
endif(XPLUG_LADSPA_SUPPORT)


if(XPLUG_VST2_SUPPORT)
add_subdirectory(vst2)
endif(XPLUG_VST2_SUPPORT)

if(XPLUG_VST3_SUPPORT)
add_subdirectory(vst3)
endif(XPLUG_VST3_SUPPORT)	


if(XPLUG_LV2_SUPPORT)
add_subdirectory(lv2)
endif(XPLUG_LV2_SUPPORT)	
