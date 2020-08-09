if(NOT BUILD_CONFIG)
SET(BUILD_CONFIG Release)
endif(NOT BUILD_CONFIG)
if(NOT BUILD_DIR)
SET(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR})
endif(NOT BUILD_DIR)
SET(SKIP_VST3 OFF)
SET(SKIP_TORTURE OFF)
SET(SKIP_LV2 OFF)
SET(SKIP_PLUGINVAL OFF)
if(WIN32)
    if(NOT INSTALL_PREFIX)
        SET(INSTALL_PREFIX "c:/Program Files" )
    endif(NOT INSTALL_PREFIX)
    SET(TORTURE_INSTALL_PREFIX ${INSTALL_PREFIX}/plugin-torture)
    SET(LV2_INSTALL_PREFIX ${INSTALL_PREFIX}/lv2)
    SET(VST3_INSTALL_PREFIX ${INSTALL_PREFIX}/vst3sdk)
    SET(PLUGINVAL_INSTALL_PREFIX ${INSTALL_PREFIX}/pluginval)
elseif(UNIX)
    if(NOT INSTALL_PREFIX)
        SET(INSTALL_PREFIX "/usr/local")
    endif(NOT INSTALL_PREFIX)
    SET(TORTURE_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(LV2_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(VST3_INSTALL_PREFIX ${INSTALL_PREFIX})
    SET(PLUGINVAL_INSTALL_PREFIX ${INSTALL_PREFIX})
endif(WIN32)



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
        get_filename_component(XINSTALL_ABS_PATH ${XINSTALL_RELATIVE_PATH} ABSOLUTE) #make relative path to absolutepath, so glob_recurse works.
        file(GLOB_RECURSE XINSTALL_FILEPATHS RELATIVE ${XINSTALL_ABS_PATH} ${XINSTALL_FILE_PATTERNS})
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
    execute_process(COMMAND git clone https://github.com/CapRat/plugin-torture.git ${BUILD_DIR}/TortureTesterSrc)
    file(MAKE_DIRECTORY ${BUILD_DIR}/TortureTesterBuild)
    execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=${BUILD_CONFIG}  -DCMAKE_INSTALL_PREFIX=${TORTURE_INSTALL_PREFIX} -S ${BUILD_DIR}/TortureTesterSrc -B ${BUILD_DIR}/TortureTesterBuild)
    execute_process(COMMAND cmake --build ${BUILD_DIR}/TortureTesterBuild --config ${BUILD_CONFIG} --target install  )
    file(REMOVE_RECURSE ${BUILD_DIR}/TortureTesterSrc)
    file(REMOVE_RECURSE ${BUILD_DIR}/TortureTesterBuild)
endif(NOT SKIP_TORTURE)

######################## INSTALLING LV2 ##########################
if(NOT SKIP_LV2)
    execute_process(COMMAND git clone  https://github.com/lv2/lv2.git ${BUILD_DIR}/LV2Src)
    xinstall(INSTALL_PREFIX ${LV2_INSTALL_PREFIX} RELATIVE_PATH ${BUILD_DIR}/LV2Src/lv2 DESTINATION include/lv2 FILE_PATTERNS ${BUILD_DIR}/LV2Src/lv2/*.h)
    file(REMOVE_RECURSE ${BUILD_DIR}/LV2Src)
endif(NOT SKIP_LV2)

######################## INSTALLING VST3 #########################
if(NOT SKIP_VST3)
    execute_process(COMMAND git clone https://github.com/steinbergmedia/vst3sdk.git ${BUILD_DIR}/Vst3SdkSrc) #Clone VST3
    execute_process(COMMAND git submodule update --init --remote pluginterfaces base cmake public.sdk WORKING_DIRECTORY ${BUILD_DIR}/Vst3SdkSrc) 
    file(REMOVE_RECURSE ${BUILD_DIR}/Vst3SdkSrc/public.sdk/samples/vst-hosting/audiohost/ ${BUILD_DIR}/Vst3SdkSrc/public.sdk/samples/vst-hosting/editorhost/)
    file(MAKE_DIRECTORY ${BUILD_DIR}/Vst3SdkBuild)
    execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=${BUILD_CONFIG} -DSMTG_ADD_VST3_PLUGINS_SAMPLES=OFF -DSMTG_RUN_VST_VALIDATOR=OFF -DSMTG_ADD_VSTGUI=OFF -S ${BUILD_DIR}/Vst3SdkSrc -B ${BUILD_DIR}/Vst3SdkBuild)  #Configure VST3
    execute_process(COMMAND cmake --build ${BUILD_DIR}/Vst3SdkBuild --config ${BUILD_CONFIG}  ) #BuildVST3

    message(STATUS "Installing files to dir ${VST3_INSTALL_PREFIX}")
    #Installing headerfiles.
    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION include/ RELATIVE_PATH ${BUILD_DIR}/Vst3SdkSrc/ FILE_PATTERNS ${BUILD_DIR}/Vst3SdkSrc/*.h)

    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION bin/ FILE_PATTERNS
        ${BUILD_DIR}/Vst3SdkBuild/bin/*/*ImageStitcher* 
        ${BUILD_DIR}/Vst3SdkBuild/bin/*/*validator*
        ${BUILD_DIR}/Vst3SdkBuild/bin/*/*uidesccompressor*
        ${BUILD_DIR}/Vst3SdkBuild/bin/*/*editorhost*)

    #Installin library files
    xinstall(INSTALL_PREFIX ${VST3_INSTALL_PREFIX} DESTINATION lib/ FILE_PATTERNS
    ${BUILD_DIR}/Vst3SdkBuild/lib/*vstgui_uidescription*  ${BUILD_DIR}/Vst3SdkBuild/lib/*vstgui*
    ${BUILD_DIR}/Vst3SdkBuild/lib/*vstgui_standalone*     ${BUILD_DIR}/Vst3SdkBuild/lib/*sdk*
    ${BUILD_DIR}/Vst3SdkBuild/lib/*vstgui_support*        ${BUILD_DIR}/Vst3SdkBuild/lib/*base*
    ${BUILD_DIR}/Vst3SdkBuild/lib/*pluginterfaces*)
    file(COPY ${BUILD_DIR}/Vst3SdkSrc/LICENSE.txt  ${BUILD_DIR}/Vst3SdkSrc/README.md DESTINATION ${VST3_INSTALL_PREFIX}) #Just to be sure, copy readme and license
    file(REMOVE_RECURSE ${BUILD_DIR}/Vst3SdkSrc)
    file(REMOVE_RECURSE ${BUILD_DIR}/Vst3SdkBuild)
endif(NOT SKIP_VST3)


#################INSTALLING PLUGINVAL######################
if(NOT SKIP_PLUGINVAL)
    if(WIN32)
        set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Windows.zip)
        set(EXE_NAME pluginval.exe)
    elseif(APPLE)
        set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_macOS.zip)
        set(EXE_NAME pluginval.app/Contents/MacOS/pluginval)
    elseif(UNIX AND NOT APPLE)
        set(PLUGINVAL_DOWNLOAD_URL https://github.com/Tracktion/pluginval/releases/download/latest_release/pluginval_Linux.zip)
        set(EXE_NAME pluginval)
    endif(WIN32)

    set(PLUGINVAL_EXE_PATH ${BUILD_DIR}/${EXE_NAME}) # Path to extracted executable
    set(PLUGINVAL_TEMP_ZIP_PATH ${BUILD_DIR}/pluginval_dload.zip) #Path to downloaded zipfile
    if(NOT EXISTS ${PLUGINVAL_EXE_PATH})
        if(NOT EXISTS ${PLUGINVAL_TEMP_ZIP_PATH})
            file(DOWNLOAD ${PLUGINVAL_DOWNLOAD_URL} ${PLUGINVAL_TEMP_ZIP_PATH})
        endif(NOT EXISTS ${PLUGINVAL_TEMP_ZIP_PATH})
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf pluginval_dload.zip RESULT_VARIABLE OUTVAR WORKING_DIRECTORY ${BUILD_DIR})
    endif(NOT EXISTS  ${PLUGINVAL_EXE_PATH})
    xinstall(INSTALL_PREFIX ${PLUGINVAL_INSTALL_PREFIX} DESTINATION bin/  FILE_PATTERNS ${PLUGINVAL_EXE_PATH})
endif(NOT SKIP_PLUGINVAL)
