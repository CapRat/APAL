find_path(VST3_DIR
    NAMES include/pluginterfaces/vst/ivstaudioprocessor.h 
    pluginterfaces/vst/ivstaudioprocessor.h 
    PATH_SUFFIXES vst3sdk plugininterfaces include
)

find_library(VST3_BASE_LIBRARY	            NAMES base PATH_SUFFIXES lib PATHS ${VST3_DIR})
find_library(VST3_SDK_LIBRARY	            NAMES sdk PATH_SUFFIXES lib PATHS ${VST3_DIR})
find_library(VST3_PLUGIN_INTERFACES_LIBRARY	NAMES pluginterfaces PATH_SUFFIXES lib PATHS ${VST3_DIR})
find_library(VST3_VSTGUI_LIBRARY	        NAMES vstgui PATH_SUFFIXES lib PATHS ${VST3_DIR})
find_program(VST3_VALIDATOR                 NAMES validator PATH_SUFFIXES bin PATHS ${VST3_DIR} )

    
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(VST3  DEFAULT_MSG  VST3_DIR VST3_BASE_LIBRARY VST3_SDK_LIBRARY VST3_PLUGIN_INTERFACES_LIBRARY VST3_VSTGUI_LIBRARY
)

if(VST3_FOUND)
  set(VST3_INCLUDE_DIRS   ${VST3_DIR}/include
  #${VST3_INCLUDE_DIR}/pluginterfaces  
  ${VST3_DIR}/include/pluginterfaces/vst)
    set(VST3_LIBRARIES ${VST3_BASE_LIBRARY} ${VST3_SDK_LIBRARY}  ${VST3_PLUGIN_INTERFACES_LIBRARY}  ${VST3_BASE_LIBRARY}  ${VST3_VSTGUI_LIBRARY})
  mark_as_advanced(VST3_INCLUDE_DIRS)
  if(NOT TARGET vst3_validator )
    add_executable(vst3_validator IMPORTED GLOBAL)
    set_target_properties(vst3_validator PROPERTIES  IMPORTED_LOCATION "${VST3_VALIDATOR}")
  endif(NOT TARGET vst3_validator )
endif(VST3_FOUND)