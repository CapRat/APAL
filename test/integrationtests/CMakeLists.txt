
function(download_target_executable DOWNLOAD_URL EXECUTABLE_FILE)

  get_filename_component(DOWNLOADED_FILE_NAME ${DOWNLOAD_URL} NAME)
  get_filename_component(EXECUTABLE_FILE_NAME ${EXECUTABLE_FILE} NAME_WE)

  set(EXECUTABLE_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_FILE})
  set(DOWNLOADED_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${DOWNLOADED_FILE_NAME})

if(NOT EXISTS ${EXECUTABLE_FILE_PATH})
  if(NOT EXISTS ${DOWNLOADED_FILE_PATH})
    file(DOWNLOAD ${DOWNLOAD_URL} ${DOWNLOADED_FILE_PATH}
   #      EXPECTED_HASH MD5=49f6a914ea33912471512ac9228c6923
   )
  endif(NOT EXISTS ${DOWNLOADED_FILE_PATH})

  # this is OS-agnostic
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf ${DOWNLOADED_FILE_PATH}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  add_executable(${EXECUTABLE_FILE_NAME} IMPORTED GLOBAL  )
  set_target_properties(${EXECUTABLE_FILE_NAME} PROPERTIES IMPORTED_LOCATION ${EXECUTABLE_FILE_PATH})
  message("Add target \"${EXECUTABLE_FILE_NAME}\"")

endif(NOT EXISTS ${EXECUTABLE_FILE_PATH})


# -- check extracted (or any other) file
#file(MD5 ${OUTFN} CHKSUM)

#if(NOT ${CHKSUM} STREQUAL 07883e93734b98cae0f7b9c55d287250)
#  message(WARNING "MD5 checksum did not match ${OUTFN}")
#endif()
endfunction(download_target_executable )



if(${XPLUG_VST2_SUPPORT} OR ${XPLUG_VST3_SUPPORT} OR ${XPLUG_AU_SUPPORT})
 SET(XPLUG_USE_PLUGINVAL ON)
if(WIN32)
    # for Windows operating system in general
    download_target_executable(https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Windows.zip pluginval.exe)
elseif(APPLE)
    # for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
    download_target_executable(https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Windows.zip pluginval.app/Contents/MacOS/pluginval)
elseif(UNIX AND NOT APPLE)
    # UNIX
    download_target_executable(https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Windows.zip pluginval)
else()
    SET(XPLUG_USE_PLUGINVAL OFF)
endif(WIN32)
endif(${XPLUG_VST2_SUPPORT} OR ${XPLUG_VST3_SUPPORT} OR ${XPLUG_AU_SUPPORT})

if(${XPLUG_USE_PLUGINVAL})
#include(integration_tests)
#run_pluginval_test(VolumePlugin)
#add_test(pluginval_test COMMAND "./pluginval --strictnessLevel 5 --validate $<TARGET_FILE:${TARGET}>" )
#add_test
endif(${XPLUG_USE_PLUGINVAL})













#############RUN The tests for Example Plugins, if they are build###############
if(${XPLUG_BUILD_EXAMPLES})
include(integration_tests)
run_pluginval_test(VolumePlugin)
endif(${XPLUG_BUILD_EXAMPLES})