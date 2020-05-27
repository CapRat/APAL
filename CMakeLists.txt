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
option(XPLUG_VST3_SUPPORT "Support vor VST3" ON)
option(XPLUG_LADSPA_SUPPORT "Support vor LADSPA" ON)
option(XPLUG_LV2_SUPPORT "Support vor LV2" ON)
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


set_target_properties(XPlug PROPERTIES POSITION_INDEPENDENT_CODE  True VERSION  ${XPlug_VERSION_MAJOR})

EXPORT_SYMBOLS(XPlug FUNCTION_NAMES "XPlugGetVersion" )
GENERATE_EXPORT_HEADER(XPlug)
GENERATE_VERSION_HEADER(XPlug VISIBILITY PRIVATE)
target_include_directories(XPlug PUBLIC  
 $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
 $<INSTALL_INTERFACE:include> )
 target_include_directories(XPlug PUBLIC  
 $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
 $<INSTALL_INTERFACE:include> )

if(XPLUG_INSTALL)

install(TARGETS XPlug EXPORT XPlugExport DESTINATION lib)
install(EXPORT XPlugExport NAMESPACE XPlug:: DESTINATION cmake)
install(FILES   cmake/FindJack.cmake
                cmake/Findpluginval.cmake
                cmake/FindTortureTester.cmake
                DESTINATION cmake)

set(CPACK_PACKAGE_NAME "XPlug")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "XPlug a Crossplattform audioplugin abstraction library")
set(CPACK_PACKAGE_DESCRIPTION "XPlug a Crossplattform audioplugin abstraction library")
set(CPACK_PACKAGE_VENDOR "Benjamin Heisch")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${XPlug_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${XPlug_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${XPlug_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "Benjamin Heisch")
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS libc6 (>= 2.7-18) )
include(CPack)

endif(XPLUG_INSTALL)
if(XPLUG_BUILD_TESTS)
enable_testing()
add_subdirectory(test)
endif(XPLUG_BUILD_TESTS)


if(XPLUG_BUILD_EXAMPLES)
add_subdirectory(examples)
endif(XPLUG_BUILD_EXAMPLES)
