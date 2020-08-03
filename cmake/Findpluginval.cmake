
find_program(pluginval_EXECUTABLE NAMES pluginval DOC "Path to pluginval tester Executable"
PATH_SUFFIXES bin/ pluginval/bin/)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pluginval
  FOUND_VAR pluginval_FOUND
  REQUIRED_VARS
    pluginval_EXECUTABLE
)

if(pluginval_FOUND AND NOT TARGET pluginval)
  add_executable(pluginval IMPORTED GLOBAL)
  set_target_properties(pluginval PROPERTIES  IMPORTED_LOCATION "${pluginval_EXECUTABLE}")
endif()