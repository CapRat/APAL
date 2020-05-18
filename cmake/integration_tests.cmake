

find_package(pluginval)
find_package(TortureTester)

# Run pluginval tests with given TARGET. pluginval must be available as target or be installed.
function(run_pluginval_test TARGET)
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
endfunction(run_pluginval_test TARGET)


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
      add_test(NAME ${TARGET}_torture_ladspa_test COMMAND $<TARGET_FILE:TortureTester> 
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


