if(XPLUG_BUILD_TESTS)
    target_include_directories(XValidate PUBLIC ${CMAKE_CURRENT_LIST_DIR}/..)
    target_sources(XValidate PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/LADSPATestSuite.cpp)
endif(XPLUG_BUILD_TESTS)