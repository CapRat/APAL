cmake_minimum_required (VERSION 3.0.0)
project(XPlug)

##########INCLUDES############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake )


#########PACKAGE INCLUDES#########
find_package(Jack)

#include(DPFFunctions) ## for Function use documentation look into DPFFuncitons or in the Examples.
include(version NO_POLICY_SCOPE)
include(cxx_handling)
include(symbol_export)
include(GenerateExportHeader)

#########Settings#############
option(XPLUG_BUILD_EXAMPLES "Build Examples" ON)
option(XPLUG_BUILD_DOC "Build Documentation" OFF)
option(XPLUG_BUILD_TESTS "Build and installs tests,which can be reused and are used in the examples." ON)
#option(XPLUG_USE_PLUGINVAL "Adds CMAKE Tests, which uses pluginval" ON)

#########################Set Supported Format Variables###########################
## Here add maybe an detect if feature is build ready macro or function.
option(XPLUG_VST2_SUPPORT "Support vor VST2" ON)
option(XPLUG_VST3_SUPPORT "Support vor VST3" OFF)
option(XPLUG_LADSPA_SUPPORT "Support vor LADSPA" ON)
option(XPLUG_LV2_SUPPORT "Support vor LV2" OFF)
option(XPLUG_AU_SUPPORT "Support vor AU" OFF)
option(XPLUG_JACK_SUPPORT "IS JACK SUPPORTED?" ${JACK_FOUND})




option(XPLUG_INSTALL "Install Xplug and Examples, if Build" ON)
set_version(1 0 0)

use_cxx(11)


#####################BUILD######################
add_subdirectory(formats)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(deps)
add_library(XPlug STATIC ${XPLUG_INCLUDE_FILES} ${XPLUG_SOURCE_FILES} )


set_target_properties(XPlug PROPERTIES POSITION_INDEPENDENT_CODE  True)
EXPORT_SYMBOLS(XPlug FUNCTION_NAMES "XPlugGetVersion" )
GENERATE_EXPORT_HEADER(XPlug)
GENERATE_VERSION_HEADER(XPlug)
target_include_directories(XPlug PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})

if(XPLUG_BUILD_TESTS)
enable_testing()
add_subdirectory(test)
endif(XPLUG_BUILD_TESTS)


if(XPLUG_BUILD_EXAMPLES)
add_subdirectory(examples)
endif(XPLUG_BUILD_EXAMPLES)