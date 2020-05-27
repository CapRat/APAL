find_package(pluginval)
find_package(TortureTester)

# Run pluginval tests with given TARGET. pluginval must be available as target or be installed.
function(run_vst2_test TARGET)
    if(TARGET pluginval)
        add_test(NAME ${TARGET}_pluginval_test COMMAND pluginval 
        --validate-in-process  
        --output-dir ${PROJECT_BINARY_DIR}/logs 
        --strictnessLevel 5
       #--skip-gui-tests
        --validate $<TARGET_FILE:${TARGET}> )
      else()
      message(WARNING "No ${TARGET}_pluginval_test is added, tue to no installed Pluginval. ")
    endif(TARGET pluginval)
endfunction(run_vst2_test TARGET)

# Run pluginval tests with given TARGET. pluginval must be available as target or be installed.
function(run_vst3_test TARGET)
    if(TARGET pluginval)
        add_test(NAME ${TARGET}_pluginval_test COMMAND pluginval
        --validate-in-process
        --output-dir ${PROJECT_BINARY_DIR}/logs
        --strictnessLevel 5
       #--skip-gui-tests
        --validate $<TARGET_FILE:${TARGET}> )
      else()
      message(WARNING "No ${TARGET}_pluginval_test is added, tue to no installed Pluginval. ")
    endif(TARGET pluginval)
endfunction(run_vst3_test TARGET)

function(run_ladspa_tests TARGET)
    if(TARGET TortureTester)
      add_test(NAME ${TARGET}_torture_ladspa_test COMMAND $<TARGET_FILE:TortureTester> 
      --evil
      --denormals
      --ladspa
      #--profile <profile file>
      --plugin  $<TARGET_FILE:${TARGET}>
      )
    endif(TARGET TortureTester)
endfunction(run_ladspa_tests TARGET)

function(run_lv2_tests TARGET)
    if(TARGET TortureTester)
      add_test(NAME ${TARGET}_torture_lv2_test COMMAND $<TARGET_FILE:TortureTester> 
      --evil
      --lv2
      --ladspa
      #--profile <profile file>
      --plugin  $<TARGET_FILE:${TARGET}>
      )
    endif(TARGET TortureTester)
endfunction(run_lv2_tests TARGET)

#function(run_pluginval_test TARGET)
#    find_package(pluginval)
#    
#    if(TARGET pluginval)
#        add_test(NAME ${TARGET}_pluginval_test COMMAND pluginval 
#        --validate-in-process  
#        --output-dir ${CMAKE_BINARY_DIR}/logs 
#        --strictnessLevel 5
#       #--skip-gui-tests
#        --validate $<TARGET_FILE:${TARGET}> )
#      else()
#      message(WARNING "No ${TARGET}_pluginval_test is added, tue to no installed Pluginval. ")
#    endif(TARGET pluginval)
#endfunction(run_pluginval_test)


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
