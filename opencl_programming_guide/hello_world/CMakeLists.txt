add_executable(hello_world main.c)
target_link_libraries(hello_world util ${OPENCL_LIBRARIES})
install(TARGETS hello_world RUNTIME DESTINATION ${BIN_DIR}/hello_world)
install(FILES hello_world.cl DESTINATION ${BIN_DIR}/hello_world)

