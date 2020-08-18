
# Add all includefiles together and set them in Parentscope
SET(APAL_INCLUDE_FILES  ${APAL_INCLUDE_FILES} ${PLUGIN_HEADER}
    ${CMAKE_CURRENT_LIST_DIR}/GlobalData.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Types.hpp

    ${CMAKE_CURRENT_LIST_DIR}/base/FeatureComponents.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/InfoComponents.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/PluginBases.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/PortComponents.hpp
    ${CMAKE_CURRENT_LIST_DIR}/base/Ports.hpp

    ${CMAKE_CURRENT_LIST_DIR}/compatibility/string_view.hpp

    ${CMAKE_CURRENT_LIST_DIR}/interfaces/IFeatureComponent.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/IInfoComponent.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/IPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/Ports/IAudioPort.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/Ports/IMidiPort.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/Ports/IPort.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/Ports/IPortComponent.hpp
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/Ports/Speaker.hpp

PARENT_SCOPE)
