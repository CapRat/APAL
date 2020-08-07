if(XPLUG_BUILD_TESTS)
    target_include_directories(XValidate SYSTEM  PUBLIC ${VST3_INCLUDE_DIRS})
    target_sources(XValidate PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/VST3TestSuite.cpp)
    cmake_policy(SET CMP0079 NEW)
    target_link_libraries(XValidate PUBLIC ${VST3_LIBRARIES})


    target_include_directories(regression_tests SYSTEM  PUBLIC ${VST3_INCLUDE_DIRS})
    target_sources(regression_tests PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/VST3RegressionTest.cpp)
    #cmake_policy(SET CMP0079 NEW)
    target_link_libraries(regression_tests PUBLIC ${VST3_LIBRARIES})
endif(XPLUG_BUILD_TESTS)