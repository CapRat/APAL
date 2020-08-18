if(APAL_BUILD_TESTS)
    target_include_directories(XValidate PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.. ${PROJECT_SOURCE_DIR}/deps/)
    target_sources(XValidate PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/LADSPATestSuite.cpp)
endif(APAL_BUILD_TESTS)