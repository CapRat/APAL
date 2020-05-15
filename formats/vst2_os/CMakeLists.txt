include(symbol_export)

add_library(xplug_vst2 "vestige.h" "vestige.cpp")
target_link_libraries(xplug_vst2 XPlug)
EXPORT_SYMBOLS(xplug_vst2 FUNCTION_NAMES VSTPluginMain )