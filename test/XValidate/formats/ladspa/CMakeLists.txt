if(XPLUG_LADSPA_SUPPORT)
    target_include_directories(XValidate PUBLIC ${CMAKE_SOURCE_DIR}/formats/ladspa)
    target_sources(XValidate PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/LADSPATestSuite.cpp)
endif(XPLUG_LADSPA_SUPPORT)