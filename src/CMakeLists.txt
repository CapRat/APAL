
# Add all includefiles together and set them in Parentscope
SET(XPLUG_SOURCE_FILES  ${XPLUG_SOURCE_FILES}
	${CMAKE_CURRENT_LIST_DIR}/GlobalData.cpp
	${CMAKE_CURRENT_LIST_DIR}/IPlugin.cpp
PARENT_SCOPE)