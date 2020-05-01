

function(EXPORT_SYMBOLS EXPORT_SYMBOLS_TARGET)
   set(oneValueArgs OUTDIR FILE_NAME)
   set(multiValueArgs FUNCTION_NAMES)
   cmake_parse_arguments(EXPORT_SYMBOLS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

   if(NOT EXPORT_SYMBOLS_OUTDIR)
        set(EXPORT_SYMBOLS_OUTDIR ${PROJECT_BINARY_DIR}/generated/)
   endif(NOT EXPORT_SYMBOLS_OUTDIR)
   if(NOT EXPORT_SYMBOLS_FILE_NAME )
        set(EXPORT_SYMBOLS_FILE_NAME ${EXPORT_SYMBOLS_TARGET}_export.def)
   endif(NOT EXPORT_SYMBOLS_FILE_NAME)
if(WIN32)
       string (REPLACE ";" "\n  " EXPORT_SYMBOLS_NEWLINED_FUNCTION_NAMES "${EXPORT_SYMBOLS_FUNCTION_NAMES}")
       file(WRITE ${EXPORT_SYMBOLS_OUTDIR}/${EXPORT_SYMBOLS_FILE_NAME} "EXPORTS \n  ${EXPORT_SYMBOLS_NEWLINED_FUNCTION_NAMES}")
       target_sources(${EXPORT_SYMBOLS_TARGET} INTERFACE  ${EXPORT_SYMBOLS_OUTDIR}/${EXPORT_SYMBOLS_FILE_NAME})
   else(WIN32)

       foreach(loop_var ${EXPORT_SYMBOLS_FUNCTION_NAMES})
            #string(APPEND SYMBOL_LINK_STRING "--require-defined=${loop_var} ")
            target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "LINKER:--undefined=${loop_var}")
          #  string(APPEND SYMBOL_LINK_STRING --undefined=${loop_var} )
       endforeach(loop_var)
      # message("linkstring: ${SYMBOL_LINK_STRING}")
      # target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "LINKER:${SYMBOL_LINK_STRING}")
       message ('target_link_options(${EXPORT_SYMBOLS_TARGET} INTERFACE "LINKER:${SYMBOL_LINK_STRING}")')
endif(WIN32)


#target_include_directories(${GENERATE_VERSION_HEADER_TARGET} PUBLIC ${GENERATE_VERSION_HEADER_OUTDIR})
endfunction(EXPORT_SYMBOLS)

