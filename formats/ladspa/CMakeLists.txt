# Enable APAL_LADSPA_SUPPORT
# if its forced to not be used by specify APAL_LADSPA_SUPPORT to OFF, dont build it.
option(APAL_LADSPA_SUPPORT "Support vor LADSPA" ON)

if(APAL_LADSPA_SUPPORT)
    include(symbol_export)
    add_library(ladspa  "ladspa.cpp")
    set_target_properties(ladspa PROPERTIES POSITION_INDEPENDENT_CODE  True)
    target_link_libraries(ladspa APAL)
    target_compile_features(ladspa PUBLIC cxx_std_11)
    target_include_directories(ladspa PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../src ${PROJECT_SOURCE_DIR}/deps)
    EXPORT_SYMBOLS(ladspa FUNCTION_NAMES ladspa_descriptor)
    install(TARGETS ladspa EXPORT APALExport DESTINATION lib)
    add_subdirectory(tests)
endif(APAL_LADSPA_SUPPORT)