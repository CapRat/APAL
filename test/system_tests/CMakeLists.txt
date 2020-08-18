add_executable(XValidate 
"XValidate.hpp"
"XValidate.cpp"
"tools/Logger.hpp"
"${PROJECT_SOURCE_DIR}/deps/clipp.hpp" 
"IFormatTestSuite.hpp" )

target_link_libraries(XValidate PUBLIC APAL)
target_include_directories(XValidate PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../test_utils ${PROJECT_SOURCE_DIR}/deps/ )



