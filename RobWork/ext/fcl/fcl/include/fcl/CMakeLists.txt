file(GLOB_RECURSE HEADERS            ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx)
file(GLOB_RECURSE CONFIGURED_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/*.h ${CMAKE_CURRENT_BINARY_DIR}/*.hxx)
set(FCL_HEADERS ${HEADERS} ${CONFIGURED_HEADERS} PARENT_SCOPE)

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" FCL_CONFIG_IN_DIR)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" FCL_CONFIG_OUT_DIR)
configure_file("${FCL_CONFIG_IN_DIR}/config.h.in" "${FCL_CONFIG_OUT_DIR}/config.h")
# workaround to have the config.h be placed somewhere accessible by RobWork within the same buildprocess
configure_file("${FCL_CONFIG_IN_DIR}/config.h.in" "${FCL_CONFIG_IN_DIR}/config.h")
# end of workaround
install(FILES "${FCL_CONFIG_OUT_DIR}/config.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fcl)
