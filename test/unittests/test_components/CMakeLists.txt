include(GenerateExportHeader)

#Static library to check if a Symbol from a Library can be forced to be loaded in shared library. This is neccessary for this Project
add_library(static_example STATIC static_example.cpp)
EXPORT_SYMBOLS(static_example FUNCTION_NAMES static_test_function static_test_function2 )

add_library(simple_symbol_export MODULE simple_symbol_export.cpp )
GENERATE_EXPORT_HEADER(simple_symbol_export)
target_link_libraries(simple_symbol_export  static_example )
target_include_directories(simple_symbol_export PUBLIC ${CMAKE_CURRENT_BINARY_DIR})