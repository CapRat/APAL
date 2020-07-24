cmake_minimum_required (VERSION 3.1.3)
project(XValidate)

add_executable(XValidate 
 "XValidate.hpp"
"XValidate.cpp"
"tools/Logger.hpp"
"tools/clipp.hpp" 
"IFormatTestSuite.hpp" 
 )
add_subdirectory(formats)

target_link_libraries(XValidate PUBLIC XPlug)