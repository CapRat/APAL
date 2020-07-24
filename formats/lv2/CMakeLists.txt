# Enable XPLUG_LV2_SUPPORT if present, othertwhise not.
# if its forced to used by specify XPLUG_LV2_SUPPORT to ON, VST3 package is required.
find_package(LV2)
option(XPLUG_LV2_SUPPORT "Support vor LV2" ${LV2_FOUND})

if(XPLUG_LV2_SUPPORT)
    include(symbol_export)

    find_package(LV2 REQUIRED)
    add_library(lv2  "lv2.cpp" "lv2_ttl_generation.cpp")
    set_target_properties(lv2 PROPERTIES POSITION_INDEPENDENT_CODE  True)
    target_link_libraries(lv2 XPlug)
    target_include_directories(lv2 PRIVATE ${LV2_INCLUDE_DIRS}  ${CMAKE_CURRENT_LIST_DIR}/../../src)
    target_compile_features(lv2 PUBLIC cxx_std_11)
    EXPORT_SYMBOLS(lv2 FUNCTION_NAMES lv2_descriptor lv2_lib_descriptor getTTLFromPlugin getManifestFromMultpleInfos getPluginInfos)
    add_executable(TTLGenerator  "TtlGenerator.cpp")
    target_link_libraries(TTLGenerator XPlug)
    install(TARGETS lv2 EXPORT XPlugExport DESTINATION lib)
   # add_executable(tt  "turtle.cpp")
endif(XPLUG_LV2_SUPPORT)