cmake_minimum_required (VERSION 2.8.8)
project(XPlug)

##########INCLUDES############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake) #Set CMake module path
#include(DPFFunctions) ## for Function use documentation look into DPFFuncitons or in the Examples.
include(version)
include(cxx_handling)
include(symbol_export)
#include(GenerateExportHeader)

#########Settings#############
option(XPLUG_BUILD_EXAMPLES "Build Examples" OFF)
option(XPLUG_BUILD_DOC "Build Documentation" OFF)
option(XPLUG_RUN_TESTS "Run XPlug tests" ON)
#option(XPLUG_INSTALL "Install Xplug and Examples, if Build" OFF)
set_version(1 0 0)
use_cxx(11)


#####################BUILD######################
add_subdirectory(formats)
add_subdirectory(include)
add_subdirectory(src)
add_library(XPlug STATIC ${XPLUG_INCLUDE_FILES} ${XPLUG_SOURCE_FILES} )
set_target_properties(XPlug PROPERTIES POSITION_INDEPENDENT_CODE  True)
EXPORT_SYMBOLS(XPlug FUNCTION_NAMES "XPlugGetVersion" )
#GENERATE_EXPORT_HEADER(XPlug)
GENERATE_VERSION_HEADER(XPlug)
target_include_directories(XPlug PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})


if(XPLUG_RUN_TESTS)
add_subdirectory(tests)
endif(XPLUG_RUN_TESTS)