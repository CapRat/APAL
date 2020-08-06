if(XPLUG_BUILD_TESTS)
   target_include_directories(XValidate PUBLIC ${LV2_INCLUDE_DIRS})
   target_sources(XValidate PUBLIC  
   ${CMAKE_CURRENT_LIST_DIR}/LV2TestSuite.cpp 
   ${CMAKE_CURRENT_LIST_DIR}/lilv.c
   )
   target_include_directories(regression_tests PUBLIC ${LV2_INCLUDE_DIRS})
   target_sources(regression_tests PUBLIC 
   ${CMAKE_CURRENT_LIST_DIR}/LV2RegressionTest.cpp
   ${CMAKE_CURRENT_LIST_DIR}/lilv.c)
endif(XPLUG_BUILD_TESTS)