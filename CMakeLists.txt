cmake_minimum_required (VERSION 3.13)
project(XPlug)

##########INCLUDES############
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)


#########PACKAGE INCLUDES#########
find_package(Jack)
find_package(Backport QUIET)

#include(DPFFunctions) ## for Function use documentation look into DPFFuncitons or in the Examples.
include(version NO_POLICY_SCOPE)
include(symbol_export)
include(GenerateExportHeader)

#########Settings#############
option(XPLUG_BUILD_EXAMPLES "Build Examples" ON)
option(XPLUG_BUILD_DOC "Build Documentation" OFF)
option(XPLUG_BUILD_TESTS "Build and installs tests,which can be reused and are used in the examples." ON)
#option(XPLUG_USE_PLUGINVAL "Adds CMAKE Tests, which uses pluginval" ON)

#########################Set Supported Format Variables###########################
## Here add maybe an detect if feature is build ready macro or function.


option(XPLUG_AU_SUPPORT "Support vor AU" OFF)
option(XPLUG_JACK_SUPPORT "IS JACK SUPPORTED?" ${JACK_FOUND})




option(XPLUG_INSTALL "Install Xplug and Examples, if Build" ON)

set_version(0 1 0)


#####################BUILD######################
if(XPLUG_BUILD_TESTS)
enable_testing()
add_subdirectory(test)
endif(XPLUG_BUILD_TESTS)

add_subdirectory(formats)

if(XPLUG_BUILD_EXAMPLES)
add_subdirectory(examples)
endif(XPLUG_BUILD_EXAMPLES)


add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(deps)


add_library(XPlug STATIC ${XPLUG_INCLUDE_FILES} ${XPLUG_SOURCE_FILES}   )

target_link_libraries(XPlug PUBLIC ${CMAKE_DL_LIBS})
set_target_properties(XPlug PROPERTIES POSITION_INDEPENDENT_CODE  True VERSION  ${XPlug_VERSION_MAJOR})
target_compile_features(XPlug PUBLIC cxx_std_14)

EXPORT_SYMBOLS(XPlug FUNCTION_NAMES "XPlugGetVersion")
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
