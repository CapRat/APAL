
#add_executable(analyse analyseplugin.c load.c "../lib_loading.cpp")
#target_include_directories(analyse PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/include/plugins)

#add_library(amp MODULE plugins/amp.c load.c "../lib_loading.cpp")
#target_include_directories(amp PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/include/plugins)
#set_target_properties(amp PROPERTIES LINKER_LANGUAGE C WINDOWS_EXPORT_ALL_SYMBOLS TRUE)