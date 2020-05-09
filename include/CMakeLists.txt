
# Add all includefiles together and set them in Parentscope
SET(XPLUG_INCLUDE_FILES  ${XPLUG_INCLUDE_FILES} ${PLUGIN_HEADER}
        ${CMAKE_CURRENT_LIST_DIR}/IPlugin.hpp
	${CMAKE_CURRENT_LIST_DIR}/GlobalData.hpp
PARENT_SCOPE)
