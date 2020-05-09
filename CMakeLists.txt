cmake_minimum_required (VERSION 3.0.0)
project(XPlug)

##########INCLUDES############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake) #Set CMake module path
#include(DPFFunctions) ## for Function use documentation look into DPFFuncitons or in the Examples.
include(version NO_POLICY_SCOPE)
include(cxx_handling)
include(symbol_export)
include(GenerateExportHeader)

#########Settings#############
option(XPLUG_BUILD_EXAMPLES "Build Examples" ON)
option(XPLUG_BUILD_DOC "Build Documentation" OFF)
option(XPLUG_BUILD_TESTS "Build and installs tests,which can be reused and are used in the examples" ON)
#option(XPLUG_USE_PLUGINVAL "Adds CMAKE Tests, which uses pluginval" ON)

option(XPLUG_VST2_SUPPORT "Support vor VST2" ON)
option(XPLUG_VST3_SUPPORT "Support vor VST3" ON)
option(XPLUG_LADSPA_SUPPORT "Support vor LADSPA" ON)
option(XPLUG_LV2_SUPPORT "Support vor LV2" ON)
option(XPLUG_AU_SUPPORT "Support vor AU" ON)

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
GENERATE_EXPORT_HEADER(XPlug)
GENERATE_VERSION_HEADER(XPlug)
target_include_directories(XPlug PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})


if(XPLUG_BUILD_EXAMPLES)
add_subdirectory(examples)
endif(XPLUG_BUILD_EXAMPLES)

if(XPLUG_BUILD_TESTS)
add_subdirectory(test)
enable_testing()
endif(XPLUG_BUILD_TESTS)
