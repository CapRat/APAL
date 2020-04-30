#########################Set Supported Format Variables###########################
find_package(Jack)
set(VST2_SUPPORT ON CACHE INTERNAL "IS VST2 SUPPORTED?")
set(LV2_SUPPORT ON CACHE INTERNAL "IS LV2 SUPPORTED?")
set(LADSPA_SUPPORT ON CACHE INTERNAL "IS LADSPA SUPPORTED?")
if(JACK_FOUND)
    set(JACK_SUPPORT ON CACHE INTERNAL "IS JACK SUPPORTED?")
endif(JACK_FOUND)

add_subdirectory(ladspa)
add_subdirectory(vst2)