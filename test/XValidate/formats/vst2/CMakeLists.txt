if(XPLUG_VST2_SUPPORT)
    target_include_directories(XValidate PUBLIC ${CMAKE_SOURCE_DIR}/formats/vst2)
    target_sources(XValidate PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/VST2TestSuite.cpp)
endif(XPLUG_VST2_SUPPORT)