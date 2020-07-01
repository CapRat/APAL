SET(BUILD_CONFIG Release)
SET(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR})
SET(SKIP_VST3 OFF)
SET(SKIP_TORTURE OFF)
SET(SKIP_LV2 OFF)
if(NOT INSTALL_PREFIX)
if(WIN32)
    SET(INSTALL_PREFIX "c:/Program Files" )
    SET(TORTURE_INSTALL_PREFIX ${INSTALL_PREFIX}/plugin-torture)
    SET(LV2_INSTALL_PREFIX ${INSTALL_PREFIX}/lv2)
    SET(VST3_INSTALL_PREFIX ${INSTALL_PREFIX}/vst3sdk)
elseif(UNIX)
    SET(INSTALL_PREFIX "/usr/local")
    SET(TORTURE_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(LV2_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(VST3_INSTALL_PREFIX ${INSTALL_PREFIX})
endif(WIN32)
endif(NOT INSTALL_PREFIX)



######################## Helper Functions #########################

#
# DESTINATION - relative path, where files should be installed to (${INSTALL_PREFIX}/DESTINATION)
# INSTALL_PREFIX - Folder, where files are installed to.
# RELATIVE_PATH - if given, the filestucture will remain, otherwise not.
# FILE_PATTERN list of filepatterns, got by GLOB_RECURSE
function(xinstall)
    set(options FORCE_OVERWRITE)
    set(oneValueArgs DESTINATION  INSTALL_PREFIX RELATIVE_PATH)
    set(multiValueArgs FILE_PATTERNS)
    cmake_parse_arguments(XINSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if(NOT XINSTALL_RELATIVE_PATH)
        file(GLOB_RECURSE XINSTALL_FILEPATHS ${XINSTALL_FILE_PATTERNS})
    else()
        file(GLOB_RECURSE XINSTALL_FILEPATHS RELATIVE ${XINSTALL_RELATIVE_PATH} ${XINSTALL_FILE_PATTERNS})
    endif(NOT XINSTALL_RELATIVE_PATH)
    foreach(XINSTALL_FILEPATH ${XINSTALL_FILEPATHS})
        if(NOT XINSTALL_RELATIVE_PATH)
            get_filename_component(XINSTALL_FILENAME ${XINSTALL_FILEPATH} NAME)
            set(XINSTALL_SRC_FILEPATH ${XINSTALL_FILEPATH})
            set(XINSTALL_DEST_FILEPATH ${XINSTALL_INSTALL_PREFIX}/${XINSTALL_DESTINATION}/${XINSTALL_FILENAME})
        else()
            set(XINSTALL_SRC_FILEPATH ${XINSTALL_RELATIVE_PATH}/${XINSTALL_FILEPATH})
            set(XINSTALL_DEST_FILEPATH ${XINSTALL_INSTALL_PREFIX}/${XINSTALL_DESTINATION}/${XINSTALL_FILEPATH})
        endif(NOT XINSTALL_RELATIVE_PATH)
        get_filename_component(XINSTALL_DEST_FILEPATH ${XINSTALL_DEST_FILEPATH} ABSOLUTE)
        get_filename_component(XINSTALL_FILEDIR  ${XINSTALL_DEST_FILEPATH} DIRECTORY)
        if(NOT EXISTS ${XINSTALL_FILEDIR})
            file(MAKE_DIRECTORY ${XINSTALL_FILEDIR})    
        endif(NOT EXISTS ${XINSTALL_FILEDIR})

        message(STATUS "Installing: ${XINSTALL_DEST_FILEPATH}")
        if(FORCE_OVERWRITE)
            file(REMOVE ${XINSTALL_DEST_FILEPATH})
        endif(FORCE_OVERWRITE)
        file(COPY ${XINSTALL_SRC_FILEPATH} DESTINATION ${XINSTALL_FILEDIR})
    endforeach(XINSTALL_FILEPATH ${XINSTALL_FILEPATHS})
endfunction(xinstall)



##################### INSTALLING TortureTester ###################
if(NOT SKIP_TORTURE)
    execute_process(COMMAND git clone https://github.com/CapRat/plugin-torture.git WORKING_DIRECTORY ${BUILD_DIR})
    file(MAKE_DIRECTORY ${BUILD_DIR}/plugin-torture/build)
    execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=${BUILD_CONFIG}  -DCMAKE_INSTALL_PREFIX=${TORTURE_INSTALL_PREFIX} .. WORKING_DIRECTORY ${BUILD_DIR}/plugin-torture/build )
    execute_process(COMMAND cmake --build . --config ${BUILD_CONFIG} --target install WORKING_DIRECTORY ${BUILD_DIR}/plugin-torture/build )
    file(REMOVE_RECURSE ${BUILD_DIR}/plugin-torture/)
endif(NOT SKIP_TORTURE)

######################## INSTALLING LV2 ##########################
if(NOT SKIP_LV2)
    execute_process(COMMAND git clone  https://github.com/lv2/lv2.git WORKING_DIRECTORY ${BUILD_DIR})
    xinstall(INSTALL_PREFIX ${LV2_INSTALL_PREFIX} RELATIVE_PATH ${BUILD_DIR}/lv2/lv2/ DESTINATION include/lv2 FILE_PATTERNS ${BUILD_DIR}/lv2/lv2/*.h)
    file(REMOVE_RECURSE ${BUILD_DIR}/lv2/)
endif(NOT SKIP_LV2)

######################## INSTALLING VST3 #########################
if(NOT SKIP_VST3)
    execute_process(COMMAND git clone --recursive https://github.com/steinbergmedia/vst3sdk.git WORKING_DIRECTORY ${BUILD_DIR}) #Clone VST3
    file(MAKE_DIRECTORY ${BUILD_DIR}/vst3sdk/build)
    execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=${BUILD_CONFIG} -DSMTG_ADD_VST3_PLUGINS_SAMPLES=OFF -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=OFF .. WORKING_DIRECTORY ${BUILD_DIR}/vst3sdk/build )  #Configure VST3
    execute_process(COMMAND cmake --build . --config ${BUILD_CONFIG}  WORKING_DIRECTORY ${BUILD_DIR}/vst3sdk/build ) #BuildVST3

    set(VST3_INSTALL_PREFIX ${INSTALL_PREFIX}/vst3sdk)
    message(STATUS "Installing files to dir ${VST3_INSTALL_PREFIX}")
    #Installing headerfiles.
    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION include/ RELATIVE_PATH ${BUILD_DIR}/vst3sdk/ FILE_PATTERNS ${BUILD_DIR}/vst3sdk/*.h)

    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION bin/ FILE_PATTERNS
        ${BUILD_DIR}/vst3sdk/build/bin/*/*ImageStitcher* 
        ${BUILD_DIR}/vst3sdk/build/bin/*/*validator*
        ${BUILD_DIR}/vst3sdk/build/bin/*/*uidesccompressor*
        ${BUILD_DIR}/vst3sdk/build/bin/*/*editorhost*)

    #Installin library files
    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION lib/ FILE_PATTERNS
    ${BUILD_DIR}/vst3sdk/build/lib/*vstgui_uidescription*  ${BUILD_DIR}/vst3sdk/build/lib/*vstgui*
    ${BUILD_DIR}/vst3sdk/build/lib/*vstgui_standalone*     ${BUILD_DIR}/vst3sdk/build/lib/*sdk*
    ${BUILD_DIR}/vst3sdk/build/lib/*vstgui_support*        ${BUILD_DIR}/vst3sdk/build/lib/*base*
    ${BUILD_DIR}/vst3sdk/build/lib/*pluginterfaces*)
    file(COPY ${BUILD_DIR}/vst3sdk/LICENSE.txt  ${BUILD_DIR}/vst3sdk/README.md DESTINATION ${VST3_INSTALL_PREFIX}) #Just to be sure, copy readme and license
    file(REMOVE_RECURSE ${BUILD_DIR}/vst3sdk/)
endif(NOT SKIP_VST3)
