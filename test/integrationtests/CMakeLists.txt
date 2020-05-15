

if(${XPLUG_BUILD_EXAMPLES})
#include(integration_tests)
#run_pluginval_test(VolumePlugin)
#add_test(pluginval_test COMMAND "./pluginval --strictnessLevel 5 --validate $<TARGET_FILE:${TARGET}>" )
#add_test
endif(${XPLUG_BUILD_EXAMPLES})


# Downloads pluginval, if available. This will not work if you operate on non x86 Architecture.


###############pluginval##################
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
        install(PROGRAMS pluginval DESTINATION bin)
     endif(XPLUG_INSTALL)
    endif(NOT TARGET pluginval)


###############TortureTester##################
    include(ExternalProject)
    ExternalProject_Add(TortureTesterBuild
      GIT_REPOSITORY    https://github.com/CapRat/plugin-torture.git
      GIT_TAG           origin/master
      CMAKE_ARGS -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
    )
    ExternalProject_Get_Property(TortureTesterBuild INSTALL_DIR)
    add_executable(TortureTester IMPORTED GLOBAL)
    set_target_properties(TortureTester PROPERTIES IMPORTED_LOCATION "${INSTALL_DIR}/bin/TortureTester")

    if(XPLUG_INSTALL)
    install(PROGRAMS TortureTester DESTINATION bin)
    install(FILES ${INSTALL_DIR}/cmake/* DESTINATION cmake)
    endif(XPLUG_INSTALL)

#############RUN The tests for Example Plugins, if they are build###############
