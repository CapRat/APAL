find_package(pluginval)
find_package(TortureTester)
find_package(VST3)
set(__DIR_OF_XPLUG_CMAKE ${CMAKE_CURRENT_LIST_DIR})
###########################HELPER_FUNCTIONS##############################
function(get_architecture)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ARCH_64_BIT_SUFFIX "_64")
        endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        if(${CMAKE_GENERATOR} MATCHES "ARM")
            set(ARCH_PREFIX "arm")
        else()
             set(ARCH_PREFIX "x86")
        endif(${CMAKE_GENERATOR} MATCHES "ARM")
        set(ARCHITECTURE_NAME ${ARCH_PREFIX}${ARCH_64_BIT_SUFFIX}-win PARENT_SCOPE)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "CrayLinuxEnvironment")
        if(CMAKE_CROSSCOMPILING) #Crosscompiling
            message(ERROR "Doesnt support crosscompileng architecture detection.")
        else()
            EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
        endif(CMAKE_CROSSCOMPILING)
        set(ARCHITECTURE_NAME ${ARCHITECTURE}-linux PARENT_SCOPE)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
        set(ARCHITECTURE_NAME ${CMAKE_ANDROID_ARCH_ABI}-android PARENT_SCOPE)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        message(ERROR "No FreeBSD Package support")
    else()
        message(ERROR "Could not detect target platform.")
    endif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
endfunction(get_architecture)

###########################PACKAGE FUNCTIONS##############################

function(create_lv2_package)
cmake_parse_arguments(CREATE_LV2_PACKAGE "" "TARGET;PATH;PACKAGE_NAME" "" ${ARGN} )
if(NOT CREATE_LV2_PACKAGE_PATH)
    set(CREATE_LV2_PACKAGE_PATH "./")
endif(NOT CREATE_LV2_PACKAGE_PATH)
if(NOT CREATE_LV2_PACKAGE_PACKAGE_NAME)
    set(CREATE_LV2_PACKAGE_PACKAGE_NAME ${TARGET}.lv2/)
endif(NOT CREATE_LV2_PACKAGE_PACKAGE_NAME)

set(LV2_PACKAGE_ROOT ${CREATE_LV2_PACKAGE_PATH}/${CREATE_LV2_PACKAGE_PACKAGE_NAME})
add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${LV2_PACKAGE_ROOT} 
    COMMAND ${CMAKE_COMMAND} -E copy  $<TARGET_FILE:${TARGET}> ${LV2_PACKAGE_ROOT}/$<TARGET_FILE_NAME:${TARGET}> 
    COMMAND TTLGenerator ARGS $<TARGET_FILE_NAME:${TARGET}> BYPRODUCTS  ${LV2_PACKAGE_ROOT}/manifest.ttl 
        ${LV2_PACKAGE_ROOT}/${TARGET}.ttl  WORKING_DIRECTORY ${LV2_PACKAGE_ROOT})
endfunction(create_lv2_package)


function(create_vst3_package)
cmake_parse_arguments(CREATE_VST3_PACKAGE "" "TARGET;PATH;PACKAGE_NAME" "" ${ARGN} )
if(NOT CREATE_VST3_PACKAGE_PATH)
    set(CREATE_VST3_PACKAGE_PATH "./")
endif(NOT CREATE_VST3_PACKAGE_PATH)
if(NOT CREATE_VST3_PACKAGE_PACKAGE_NAME)
    set(CREATE_VST3_PACKAGE_PACKAGE_NAME ${TARGET})
endif(NOT CREATE_VST3_PACKAGE_PACKAGE_NAME)

set(VST3_PACKAGE_ROOT ${CREATE_VST3_PACKAGE_PATH}/${CREATE_VST3_PACKAGE_PACKAGE_NAME})
get_architecture()
add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${VST3_PACKAGE_ROOT} 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VST3_PACKAGE_ROOT}/Contents/Resources 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${VST3_PACKAGE_ROOT}/Contents/${ARCHITECTURE_NAME} 
    COMMAND ${CMAKE_COMMAND} -E copy  $<TARGET_FILE:${TARGET}> ${VST3_PACKAGE_ROOT}/Contents/${ARCHITECTURE_NAME}/$<TARGET_FILE_BASE_NAME:${TARGET}>.vst3
    COMMAND ${CMAKE_COMMAND} -E copy  ${__DIR_OF_XPLUG_CMAKE}/speaker.ico ${VST3_PACKAGE_ROOT}/Plugin.ico)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
    file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated_desktop.ini CONTENT "[.ShellClassInfo]\nIconResource=Plugin.ico,0")
    add_custom_command(TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy  ${CMAKE_CURRENT_BINARY_DIR}/generated_desktop.ini ${VST3_PACKAGE_ROOT}/desktop.ini
        COMMAND attrib +s +r +h ${VST3_PACKAGE_ROOT}/desktop.ini
        COMMAND attrib +r +h ${VST3_PACKAGE_ROOT}/Plugin.ico)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows" OR ${CMAKE_SYSTEM_NAME} STREQUAL "MSYS")
endfunction(create_vst3_package)



################################TEST FUNCTIONS##########################
# Run pluginval tests with given TARGET. pluginval must be available as target or be installed.
function(run_vst2_test TARGET)
    if(TARGET pluginval)
        add_test(NAME ${TARGET}_pluginval_vst2_test COMMAND pluginval " \"  --validate-in-process --validate $<TARGET_FILE:${TARGET}> --strictness-level 10 \"" )
      else()
      message(WARNING "No ${TARGET}_pluginval_test is added, due to no installed Pluginval. ")
    endif(TARGET pluginval)
    add_test(NAME ${TARGET}_xvalidate_vst2_test COMMAND XValidate -vst2 -l 10 -p $<TARGET_FILE:${TARGET}> )
endfunction(run_vst2_test TARGET)

# Run pluginval tests with given TARGET. pluginval must be available as target or be installed.
function(run_vst3_test TARGET)
    create_vst3_package(TARGET ${TARGET} PATH ${CMAKE_CURRENT_BINARY_DIR}/temp/ PACKAGE_NAME ${TARGET}.vst3)
    if(TARGET pluginval)
       add_test(NAME ${TARGET}_pluginval_vst3_test COMMAND pluginval " \"  --validate-in-process --validate ${CMAKE_CURRENT_BINARY_DIR}/temp/${TARGET}.vst3 --strictness-level 10 \"" )
    else()
      message(WARNING "No ${TARGET}_pluginval_test is added, tue to no installed Pluginval. ")
    endif(TARGET pluginval)
    
    if(TARGET vst3_validator)
         add_test(NAME ${TARGET}_vst3_validator_test COMMAND vst3_validator -e ${CMAKE_CURRENT_BINARY_DIR}/temp/${TARGET}.vst3  )
      else()
      message(WARNING "No ${TARGET}_vst3_validator_test is added, tue to no installed vst3validator. ")
    endif(TARGET vst3_validator)

    add_test(NAME ${TARGET}_xvalidate_vst3_test COMMAND XValidate -vst3 -l 10 -p $<TARGET_FILE:${TARGET}> )
endfunction(run_vst3_test TARGET)

function(run_ladspa_tests TARGET)
    if(TARGET TortureTester)
      add_test(NAME ${TARGET}_torture_ladspa_test COMMAND $<TARGET_FILE:TortureTester> --evil --denormals --ladspa --plugin  $<TARGET_FILE:${TARGET}>)
    endif(TARGET TortureTester)

    add_test(NAME ${TARGET}_xvalidate_ladspa_test COMMAND XValidate -ladspa -l 10 -p $<TARGET_FILE:${TARGET}> )
endfunction(run_ladspa_tests TARGET)

function(run_lv2_tests TARGET)
    
    create_lv2_package(TARGET ${TARGET} PATH ${CMAKE_CURRENT_BINARY_DIR}/temp/ PACKAGE_NAME ${TARGET}.lv2)
    set(LV2_TEMP_PACKAGE ${CMAKE_CURRENT_BINARY_DIR}/temp/${TARGET}.lv2/)
    if(TARGET TortureTester)
      add_test(NAME ${TARGET}_torture_lv2_test COMMAND $<TARGET_FILE:TortureTester>  --evil --lv2 --plugin  ${LV2_TEMP_PACKAGE}/$<TARGET_FILE_NAME:${TARGET}> )
    endif(TARGET TortureTester)



    add_test(NAME ${TARGET}_xvalidate_lv2_test COMMAND XValidate -lv2 -l 10 -p  ${LV2_TEMP_PACKAGE}/$<TARGET_FILE_NAME:${TARGET}> )
endfunction(run_lv2_tests TARGET)


macro(DLOAD_PLUGINVAL)
    if(NOT TARGET pluginval)
    if(WIN32)
        set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Windows.zip)
        set(EXE_NAME pluginval.exe)
    elseif(APPLE)
         set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_macOS.zip)
         set(EXE_NAME pluginval.app/Contents/MacOS/pluginval)
    elseif(UNIX AND NOT APPLE)
         set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Linux.zip)
         set(EXE_NAME pluginval)
    endif(WIN32)

    set(PLUGINVAL_EXE_PATH ${CMAKE_BINARY_DIR}/${EXE_NAME}) # Path to extracted executable
    set(PLUGINVAL_TEMP_ZIP_PATH ${CMAKE_BINARY_DIR}/pluginval_dload.zip) #Path to downloaded zipfile
     if(NOT EXISTS ${PLUGINVAL_EXE_PATH})
       if(NOT EXISTS ${PLUGINVAL_TEMP_ZIP_PATH})
        file(DOWNLOAD ${PLUGINVAL_DOWNLOAD_URL} ${PLUGINVAL_TEMP_ZIP_PATH})
       endif(NOT EXISTS ${PLUGINVAL_TEMP_ZIP_PATH})
       execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${PLUGINVAL_TEMP_ZIP_PATH} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
     endif(NOT EXISTS  ${PLUGINVAL_EXE_PATH})
     add_executable(pluginval IMPORTED GLOBAL)
     set_target_properties(pluginval PROPERTIES IMPORTED_LOCATION "${PLUGINVAL_EXE_PATH}")

     if(XPLUG_INSTALL)
        install(PROGRAMS  "${PLUGINVAL_EXE_PATH}" DESTINATION bin)
     endif(XPLUG_INSTALL)
 endif(NOT TARGET pluginval)
 endmacro(DLOAD_PLUGINVAL)
