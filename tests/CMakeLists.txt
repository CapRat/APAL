###includes###
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/Catch2/contrib) #Set CMake module path
include(CTest)
include(Catch)
include(ParseAndAddCatchTests)
include(GenerateExportHeader)

enable_testing()

add_subdirectory(ladspa)
add_subdirectory(Catch2)

#Compile test shared library to test dynamic symbol loading
add_library(ExportLib MODULE "TestSymbolLoading.cpp" )
target_compile_definitions(ExportLib PRIVATE EXPORT_SYMBOL_LOADING)
set_target_properties(ExportLib PROPERTIES  WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
GENERATE_EXPORT_HEADER(ExportLib)
target_link_libraries(ExportLib  XPlug )
target_include_directories(ExportLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})




add_executable(catch_tests catch_main.cpp "TestSymbolLoading.cpp" "TestLadspa.cpp" "lib_loading.cpp" "catch_main.cpp" )
target_include_directories(catch_tests PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(catch_tests Catch2::Catch2 XPlug)
#add_dependencies(catch_tests SymbolExportLib)#Add Dependency, so that the exportlib is build before the testlib



ParseAndAddCatchTests(catch_tests)
#catch_discover_tests(catch_tests)

#target_link_libraries(tests Catch2::Catch2)