# Enable XPLUG_LADSPA_SUPPORT
# if its forced to not be used by specify XPLUG_LADSPA_SUPPORT to OFF, dont build it.
option(XPLUG_LADSPA_SUPPORT "Support vor LADSPA" ON)

if(XPLUG_LADSPA_SUPPORT)
    include(symbol_export)
    add_library(ladspa  "ladspa.h" "ladspa.cpp")
    set_target_properties(ladspa PROPERTIES POSITION_INDEPENDENT_CODE  True)
    target_link_libraries(ladspa XPlug)
    target_compile_features(ladspa PUBLIC cxx_std_11)
    target_include_directories(ladspa PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../src)
    EXPORT_SYMBOLS(ladspa FUNCTION_NAMES ladspa_descriptor)
    install(TARGETS ladspa EXPORT XPlugExport DESTINATION lib)
    add_subdirectory(tests)
endif(XPLUG_LADSPA_SUPPORT)