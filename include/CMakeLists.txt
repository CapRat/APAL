
# Add all includefiles together and set them in Parentscope
SET(XPLUG_INCLUDE_FILES  ${XPLUG_INCLUDE_FILES} ${PLUGIN_HEADER}
    ${CMAKE_CURRENT_LIST_DIR}/GlobalData.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Types.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/IPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/IPortComponent.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/LazyPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/SimplePortComponent.hpp
PARENT_SCOPE)
