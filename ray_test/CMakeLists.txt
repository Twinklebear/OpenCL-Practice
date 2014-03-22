set(CL_PROGRAM_DIR "${BIN_DIR}/ray_test/")
configure_file(cl_program_dir.h.in cl_program_dir.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(ray_test main.c)
target_link_libraries(ray_test util ${OPENCL_LIBRARIES})
install(TARGETS ray_test RUNTIME DESTINATION ${BIN_DIR}/ray_test)
install(FILES ray_test.cl DESTINATION ${BIN_DIR}/ray_test)

