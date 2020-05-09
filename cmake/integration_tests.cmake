
function(run_pluginval_test TARGET)
if(NOT TARGET pluginval)
    find_package(pluginval)
endif(NOT TARGET pluginval)
add_test(NAME pluginval_test COMMAND pluginval --strictnessLevel 5 --validate $<TARGET_FILE:${TARGET}> )
endfunction(run_pluginval_test)