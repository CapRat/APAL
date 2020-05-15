include(Catch)
include(ParseAndAddCatchTests)


add_subdirectory(test_components)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp CONTENT "#define CATCH_CONFIG_MAIN\r\n#include \"catch2/catch.hpp\"" )

add_executable(catch_tests ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp ${TEST_TOOLS_SOURCE}
    "TestSymbolLoading.cpp"
    "TestBaseFunctionality.cpp"
    "./test_components/simple_example_plugin.cpp")

target_include_directories(catch_tests PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/../tools/ ${CMAKE_CURRENT_LIST_DIR}/../../examples/Volume Plugin)

if (NOT WIN32)
    SET(ADD_LIBS ${ADD_LIBS} dl)
else()
endif(NOT WIN32)

target_link_libraries(catch_tests Catch2::Catch2 XPlug ${ADD_LIBS})

ParseAndAddCatchTests(catch_tests)
#add_dependencies(catch_tests SymbolExportLib)#Add Dependency, so that the exportlib is build before the testlib




#catch_discover_tests(catch_tests)

#target_link_libraries(tests Catch2::Catch2)
