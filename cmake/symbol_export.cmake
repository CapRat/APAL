

function(EXPORT_SYMBOLS EXPORT_SYMBOLS_TARGET)
   set(oneValueArgs )
   set(multiValueArgs FUNCTION_NAMES)
   cmake_parse_arguments(EXPORT_SYMBOLS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
   foreach(loop_var ${EXPORT_SYMBOLS_FUNCTION_NAMES})
   if(WIN32)
     target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "/EXPORT:${loop_var}")
   else(WIN32)
     target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "LINKER:--undefined=${loop_var}")
   endif(WIN32)
   endforeach(loop_var)

endfunction(EXPORT_SYMBOLS)

