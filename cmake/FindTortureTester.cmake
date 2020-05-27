#TortureTester_EXECUTABLE
find_program(TortureTester_EXECUTABLE NAMES plugin-torture torture TortureTester DOC "Path to torture-plugin tester Executable"
PATHS bin)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TortureTester
  FOUND_VAR TortureTester_FOUND
  REQUIRED_VARS
    TortureTester_EXECUTABLE
)

if(TortureTester_FOUND AND NOT TARGET TortureTester)
  add_executable(TortureTester IMPORTED GLOBAL)
  set_target_properties(TortureTester PROPERTIES  IMPORTED_LOCATION "${TortureTester_EXECUTABLE}")
endif()