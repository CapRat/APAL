set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/Catch2/contrib PARENT_SCOPE) #Set CMake module path
add_subdirectory(Catch2)

#file(GLOB plugin_torture_src lv2bm/src/*.cpp)
#add_executable(torture_plugin_tester  ${plugin_torture_src} ${CMAKE_CURRENT_LIST_DIR}/../test/tools/lib_loading.cpp)
#target_include_directories(torture_plugin_tester PUBLIC 
#${CMAKE_CURRENT_LIST_DIR}/../test/tools/ 
#${CMAKE_CURRENT_LIST_DIR}/../formats/ladspa
#${CMAKE_CURRENT_LIST_DIR}/lv2/lv2
#)

