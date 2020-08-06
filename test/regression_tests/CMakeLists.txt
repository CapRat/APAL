file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp CONTENT "#define CATCH_CONFIG_MAIN\r\n#include \"catch.hpp\"" )
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/regression_data.hpp CONTENT 
"#define MIDI_FORWARDER \"$<TARGET_FILE:MidiForwarder>\"\r\n#define VOLUME_PLUGIN \"$<TARGET_FILE:VolumePlugin>\"\r\n" )
add_executable(regression_tests ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp)

target_include_directories(regression_tests PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../test_utils ../test_utils   ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(regression_tests PUBLIC XPlug)
add_test(NAME xplug_regression_tests COMMAND regression_tests)