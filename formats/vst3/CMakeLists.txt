# Enable XPLUG_VST3_SUPPORT if present, othertwhise not.
# if its forced to used by specify XPLUG_VST3_SUPPORT to ON, VST3 package is required.
find_package(VST3)
option(XPLUG_VST3_SUPPORT "Support vor VST3" ${VST3_FOUND})

if(XPLUG_VST3_SUPPORT)
	include(symbol_export)
	#if VST3 Support is build, Required is true.
	find_package(VST3 REQUIRED)
	add_library(vst3 vst3.cpp Component.cpp VST3AudioProcessorImpl.cpp VST3EditControllerImpl.cpp)
	set_target_properties(vst3 PROPERTIES POSITION_INDEPENDENT_CODE  True)
	target_link_libraries(vst3 XPlug ${VST3_LIBRARIES})
	target_include_directories(vst3 PRIVATE ${VST3_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/../../src)

	target_compile_features(vst3 PUBLIC cxx_std_11)
	if(WIN32)
		EXPORT_SYMBOLS(vst3 FUNCTION_NAMES 	GetPluginFactory InitDll ExitDll)
	elseif(APPLE)
		# for MacOS X or iOS, watchOS, tvOS (since 3.10.3)
		EXPORT_SYMBOLS(vst3 FUNCTION_NAMES 	_GetPluginFactory _bundleEntry _bundleExit)
	elseif(UNIX AND NOT APPLE)
                EXPORT_SYMBOLS(vst3 FUNCTION_NAMES 	GetPluginFactory ModuleEntry ModuleExit  InitDll ExitDll)
	else()
	MESSAGE()
	endif(WIN32)

	install(TARGETS vst3 EXPORT XPlugExport DESTINATION lib)
endif(XPLUG_VST3_SUPPORT)
