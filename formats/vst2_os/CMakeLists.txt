include(symbol_export)

add_library(vst2 "vestige.h" "vestige.cpp")
set_target_properties(vst2 PROPERTIES POSITION_INDEPENDENT_CODE  True)
target_link_libraries(vst2 XPlug)
EXPORT_SYMBOLS(vst2 FUNCTION_NAMES VSTPluginMain )
install(TARGETS vst2 EXPORT XPlugExport DESTINATION lib)