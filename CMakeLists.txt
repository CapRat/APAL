cmake_minimum_required (VERSION 3.13)
project(APAL)

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
option(APAL_BUILD_EXAMPLES "Build Examples" ON)
option(APAL_BUILD_DOC "Build Documentation" OFF)
option(APAL_BUILD_TESTS "Build and installs tests,which can be reused and are used in the examples." ON)
#option(APAL_USE_PLUGINVAL "Adds CMAKE Tests, which uses pluginval" ON)

#########################Set Supported Format Variables###########################
## Here add maybe an detect if feature is build ready macro or function.


option(APAL_AU_SUPPORT "Support vor AU" OFF)
option(APAL_JACK_SUPPORT "IS JACK SUPPORTED?" ${JACK_FOUND})




option(APAL_INSTALL "Install APAL and Examples, if Build" ON)

set_version(0 1 0)


#####################BUILD######################
if(APAL_BUILD_TESTS)
enable_testing()
add_subdirectory(test)
endif(APAL_BUILD_TESTS)

add_subdirectory(formats)

if(APAL_BUILD_EXAMPLES)
add_subdirectory(examples)
endif(APAL_BUILD_EXAMPLES)


add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(deps)


add_library(APAL STATIC ${APAL_INCLUDE_FILES} ${APAL_SOURCE_FILES}   )

target_link_libraries(APAL PUBLIC ${CMAKE_DL_LIBS})
set_target_properties(APAL PROPERTIES POSITION_INDEPENDENT_CODE  True VERSION  ${APAL_VERSION_MAJOR})
target_compile_features(APAL PUBLIC cxx_std_14)

EXPORT_SYMBOLS(APAL FUNCTION_NAMES "APALGetVersion")
GENERATE_EXPORT_HEADER(APAL)
GENERATE_VERSION_HEADER(APAL VISIBILITY PRIVATE)
target_include_directories(APAL PUBLIC  
 $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
 $<INSTALL_INTERFACE:include> )
 target_include_directories(APAL PUBLIC  
 $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
 $<INSTALL_INTERFACE:include> )

if(APAL_INSTALL)

install(TARGETS APAL EXPORT APALExport DESTINATION lib)
install(EXPORT APALExport NAMESPACE APAL:: DESTINATION cmake)
install(FILES   cmake/FindJack.cmake
                cmake/Findpluginval.cmake
                cmake/FindTortureTester.cmake
                DESTINATION cmake)

set(CPACK_PACKAGE_NAME "APAL")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "APAL a Crossplattform audioplugin abstraction library")
set(CPACK_PACKAGE_DESCRIPTION "APAL a Crossplattform audioplugin abstraction library")
set(CPACK_PACKAGE_VENDOR "Benjamin Heisch")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${APAL_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${APAL_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${APAL_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "Benjamin Heisch")
#set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS libc6 (>= 2.7-18) )
include(CPack)

endif(APAL_INSTALL)
