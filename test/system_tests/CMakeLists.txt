add_executable(XValidate 
 "XValidate.hpp"
"XValidate.cpp"
"tools/Logger.hpp"
"tools/clipp.hpp" 
"IFormatTestSuite.hpp" )

target_link_libraries(XValidate PUBLIC XPlug)
target_include_directories(XValidate PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../test_utils )



