include(${CMAKE_SOURCE_DIR}/cmake/integration_tests.cmake)

add_library(VolumePlugin MODULE VolumePlugin.cpp )
target_link_libraries(VolumePlugin PUBLIC  XPlug xplug_ladspa xplug_vst2)