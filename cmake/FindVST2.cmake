find_path(VST2_INCLUDE_DIR
    NAMES aeffect.h 
    PATHS ${CMAKE_CURRENT_LIST_DIR}/../deps
    PATH_SUFFIXES vstsdk2.4 vstsdk2.4/pluginterfaces/vst2.x include/pluginterfaces/vst2.x
)

    
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(VST2  DEFAULT_MSG VST2_INCLUDE_DIR)

if(VST2_FOUND)
  set(VST2_INCLUDE_DIRS  
  ${VST2_INCLUDE_DIR}
  ${VST3_INCLUDE_DIR}/pluginterfaces
  )
  mark_as_advanced(VST2_INCLUDE_DIRS)
  
endif(VST2_FOUND)
