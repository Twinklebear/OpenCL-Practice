set(CL_PROGRAM_DIR "${BIN_DIR}/hello_world/")
configure_file(cl_program_dir.h.in cl_program_dir.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(hello_world main.c)
target_link_libraries(hello_world util ${OPENCL_LIBRARIES})
install(TARGETS hello_world RUNTIME DESTINATION ${BIN_DIR}/hello_world)
install(FILES hello_world.cl DESTINATION ${BIN_DIR}/hello_world)

