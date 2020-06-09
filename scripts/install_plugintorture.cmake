# Clone and install plugin-torture
SET(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR})
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

if(INSTALL_DIR)
SET(INSTALL_VARIABLE -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
endif(INSTALL_DIR)
execute_process(COMMAND git clone https://github.com/CapRat/plugin-torture.git WORKING_DIRECTORY ${BUILD_DIR})
file(MAKE_DIRECTORY ${BUILD_DIR}/plugin-torture/build)
execute_process(COMMAND cmake -DCMAKE_BUILD_TYPE=Release ${INSTALL_VARIABLE} .. WORKING_DIRECTORY ${BUILD_DIR}/plugin-torture/build )
execute_process(COMMAND cmake --build . --config Release --target install WORKING_DIRECTORY ${BUILD_DIR}/plugin-torture/build )

