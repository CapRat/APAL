include(symbol_export)

add_library(ladspa  "ladspa.h" "ladspa.cpp")
set_target_properties(ladspa PROPERTIES POSITION_INDEPENDENT_CODE  True)
target_link_libraries(ladspa XPlug)
target_compile_features(ladspa PUBLIC cxx_std_11)
EXPORT_SYMBOLS(ladspa FUNCTION_NAMES ladspa_descriptor)
install(TARGETS ladspa EXPORT XPlugExport DESTINATION lib)