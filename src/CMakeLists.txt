
# Add all includefiles together and set them in Parentscope
SET(XPLUG_SOURCE_FILES  ${XPLUG_SOURCE_FILES}
    ${CMAKE_CURRENT_LIST_DIR}/GlobalData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools/LibLoading.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools/PluginUtils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base/SimplePortComponent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/base/LazyPlugin.cpp
PARENT_SCOPE)