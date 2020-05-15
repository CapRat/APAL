

add_library(VolumePlugin MODULE VolumePlugin.cpp )
target_link_libraries(VolumePlugin PUBLIC  XPlug xplug_ladspa xplug_vst2)

if(XPLUG_BUILD_TESTS)
include(integration_tests)
run_pluginval_test(VolumePlugin)
RUN_LADSPA_TESTS(VolumePlugin)
endif(XPLUG_BUILD_TESTS)
