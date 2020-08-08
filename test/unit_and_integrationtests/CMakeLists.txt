include(GenerateExportHeader)

#Static library to check if a Symbol from a Library can be forced to be loaded in shared library. This is neccessary for this Project
add_library(static_example STATIC "tools/TestLibLoading.cpp")
EXPORT_SYMBOLS(static_example FUNCTION_NAMES static_test_function static_test_function2 )
target_compile_definitions(static_example PRIVATE BUILD_TEST_LIB BUILD_STATIC_EXAMPLE)
set_target_properties(static_example PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_library(simple_symbol_export MODULE "tools/TestLibLoading.cpp" )
target_compile_definitions(simple_symbol_export PRIVATE BUILD_TEST_LIB BUILD_SIMPLE_SYMBOL_EXPORT)
GENERATE_EXPORT_HEADER(simple_symbol_export)
target_link_libraries(simple_symbol_export PRIVATE static_example )
target_include_directories(simple_symbol_export PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp CONTENT "#define CATCH_CONFIG_MAIN\r\n#include \"catch.hpp\"" )

add_executable(unit_tests ${CMAKE_CURRENT_BINARY_DIR}/catch_main.cpp
    "interfaces/InterfaceTestMethods.cpp"
    "tools/TestLibLoading.cpp"
    "tools/TestPortHandling.cpp"
    "tools/TestStringTools.cpp"
    "base/TestFeatureComponents.cpp"
    "base/TestPortComponents.cpp"
    "base/TestInfoComponents.cpp"
    "base/TestPorts.cpp"
    "base/TestPluginBases.cpp"
    "TestGlobalData.cpp"
    )
target_include_directories(unit_tests PUBLIC "${CMAKE_CURRENT_LIST_DIR}" "${CMAKE_CURRENT_LIST_DIR}/../test_utils" "../test_utils")

if (NOT WIN32)
    SET(ADD_LIBS ${ADD_LIBS} dl)
else()
endif(NOT WIN32)

target_link_libraries(unit_tests XPlug ${ADD_LIBS})
add_test(NAME xplug_unit_tests COMMAND unit_tests)

#add_dependencies(unit_tests SymbolExportLib)#Add Dependency, so that the exportlib is build before the testlib




#catch_discover_tests(unit_tests)

#target_link_libraries(tests Catch2::Catch2)
