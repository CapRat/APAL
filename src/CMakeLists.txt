
# Add all includefiles together and set them in Parentscope
SET(XPLUG_SOURCE_FILES  ${XPLUG_SOURCE_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/GlobalData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools/LibLoading.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools/PortHandling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools/StringTools.cpp
 #   ${CMAKE_CURRENT_LIST_DIR}/base/PortComponents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base/FeatureComponents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base/InfoComponents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base/PluginBases.cpp

PARENT_SCOPE)