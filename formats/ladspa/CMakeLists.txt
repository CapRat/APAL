include(symbol_export)

add_library(xplug_ladspa  "ladspa.h" "ladspa.cpp")
target_link_libraries(xplug_ladspa XPlug)
EXPORT_SYMBOLS(xplug_ladspa FUNCTION_NAMES ladspa_descriptor)