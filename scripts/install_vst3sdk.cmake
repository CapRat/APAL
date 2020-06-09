# Clone and installs the Vst3Sdk
# call this script with cmake [-DBUILD_DIR={BUILD_DIR} -DINSTALL_PREFIX={INSTALL_PREFIX} -DBUILD_CONFIG={BUILD_CONFIG}] -P ./install_vst3sdk.cmake
# BUILD_DIR = filepath to clone and build vst3sdk TortureTester. Defaults to the dir of the current list file.
# INSTALL_PREFIX = filepath to install the vst3sdk folder to.
# BUILD_CONFIG = Config to build and install. Defaults to Release
SET(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR})
SET(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BUILD_CONFIG Debug)
if(NOT INSTALL_PREFIX)
if(WIN32)
SET(INSTALL_PREFIX "c:/Program Files")
elseif(UNIX)
SET(INSTALL_PREFIX "/usr/local")
endif(WIN32)
endif(NOT INSTALL_PREFIX)
execute_process(COMMAND git clone --recursive https://github.com/steinbergmedia/vst3sdk.git WORKING_DIRECTORY ${BUILD_DIR} COMMAND_ECHO STDOUT) 
file(MAKE_DIRECTORY ${BUILD_DIR}/vst3sdk/build)
execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=${BUILD_CONFIG} .. WORKING_DIRECTORY ${BUILD_DIR}/vst3sdk/build COMMAND_ECHO STDERR) 
execute_process(COMMAND cmake --build . --config ${BUILD_CONFIG}  WORKING_DIRECTORY ${BUILD_DIR}/vst3sdk/build COMMAND_ECHO STDERR) 

set(INSTALL_DIR ${INSTALL_PREFIX}/vst3sdk)

message(STATUS "Installing files to dir ${INSTALL_DIR}")
file(GLOB_RECURSE VST3_EXECUTABLES  
#${BUILD_DIR}/vst3sdk/build/bin/*[.exe]? )
	${BUILD_DIR}/vst3sdk/build/bin/*/*ImageStitcher* 
	${BUILD_DIR}/vst3sdk/build/bin/*/*validator*
	${BUILD_DIR}/vst3sdk/build/bin/*/*uidesccompressor*
	${BUILD_DIR}/vst3sdk/build/bin/*/*editorhost*)

#Installin executable files
foreach(VST3_EXECUTABLE ${VST3_EXECUTABLES})
	get_filename_component(VST3_EXECUTABLE_FILENAME ${VST3_EXECUTABLE} NAME)
	message(STATUS "Installing: ${INSTALL_DIR}/bin/${VST3_EXECUTABLE_FILENAME}")
	file(COPY ${VST3_EXECUTABLE} DESTINATION  ${INSTALL_DIR}/bin )
endforeach(VST3_EXECUTABLE ${VST3_EXECUTABLES})
	
#Installin library files
file(GLOB_RECURSE VST3_LIBRARIES 
${BUILD_DIR}/vst3sdk/build/lib/*vstgui_uidescription*	${BUILD_DIR}/vst3sdk/build/lib/*vstgui*
${BUILD_DIR}/vst3sdk/build/lib/*vstgui_standalone*		${BUILD_DIR}/vst3sdk/build/lib/*sdk*
${BUILD_DIR}/vst3sdk/build/lib/*vstgui_support*			${BUILD_DIR}/vst3sdk/build/lib/*base*
${BUILD_DIR}/vst3sdk/build/lib/*pluginterfaces*)

foreach(VST3_LIBRARY ${VST3_LIBRARIES})
	get_filename_component(VST3_LIBRARY_FILENAME ${VST3_LIBRARY} NAME)
	message(STATUS "Installing: ${INSTALL_DIR}/lib/${VST3_LIBRARY_FILENAME}")
	file(COPY  ${VST3_LIBRARY} DESTINATION  ${INSTALL_DIR}/lib )
endforeach(VST3_LIBRARY ${VST3_LIBRARYS})

file(COPY ${BUILD_DIR}/vst3sdk/pluginterfaces
 ${BUILD_DIR}/vst3sdk/public.sdk
 ${BUILD_DIR}/vst3sdk/vstgui4
 ${BUILD_DIR}/vst3sdk/base
DESTINATION ${INSTALL_DIR}/include FILES_MATCHING PATTERN *.h)

file(COPY ${BUILD_DIR}/vst3sdk/LICENSE.txt  ${BUILD_DIR}/vst3sdk/Readme.md
#${BUILD_DIR}/vst3sdk/doc 
DESTINATION ${INSTALL_DIR})

#file(COPY ${BUILD_DIR}/vst3sdk/build/)

