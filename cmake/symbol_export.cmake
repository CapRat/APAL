

function(EXPORT_SYMBOLS EXPORT_SYMBOLS_TARGET)
   set(oneValueArgs )
   set(multiValueArgs FUNCTION_NAMES)
   cmake_parse_arguments(EXPORT_SYMBOLS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
   cmake_policy(SET CMP0054 NEW)
   foreach(loop_var ${EXPORT_SYMBOLS_FUNCTION_NAMES})
       if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")
         target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "LINKER:--undefined=${loop_var}")
       elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
         #MESSAGE("GNU")
       elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
         MESSAGE("No Symbol export defined. Specify here, how to handle")
       elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
         target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "/EXPORT:${loop_var}")
       else()
          MESSAGE("${CMAKE_CXX_COMPILER_ID} Compiler not supported. Add Way to force symbol export per cmd here and it will works.")
       endif()
   endforeach(loop_var)

endfunction(EXPORT_SYMBOLS)

