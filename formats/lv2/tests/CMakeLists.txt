if(APAL_BUILD_TESTS)
   target_include_directories(XValidate SYSTEM PUBLIC ${LV2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/deps/)
   target_sources(XValidate PUBLIC  
   ${CMAKE_CURRENT_LIST_DIR}/LV2TestSuite.cpp 
   ${PROJECT_SOURCE_DIR}/deps/lilv.c
   )
   target_include_directories(regression_tests SYSTEM  PUBLIC ${LV2_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/deps/)
   target_sources(regression_tests PUBLIC 
   ${CMAKE_CURRENT_LIST_DIR}/LV2RegressionTest.cpp
   ${PROJECT_SOURCE_DIR}/deps/lilv.c)
endif(APAL_BUILD_TESTS)