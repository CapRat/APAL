  find_path(LV2_INCLUDE_DIR
      NAMES core/lv2.h
      PATHS ${CMAKE_CURRENT_LIST_DIR}/../deps/lv2
      PATH_SUFFIXES include lv2 include core
  )

find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(LV2  DEFAULT_MSG  LV2_INCLUDE_DIR)


if(LV2_FOUND)
  set(LV2_INCLUDE_DIRS  ${LV2_INCLUDE_DIR})
  mark_as_advanced(LV2_INCLUDE_DIRS)
endif(LV2_FOUND)
