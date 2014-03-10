set(CL_PROGRAM_DIR "${BIN_DIR}/ch2_simple_convolution/")
configure_file(cl_program_dir.h.in cl_program_dir.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(ch2_simple_convolution main.c)
target_link_libraries(ch2_simple_convolution util ${OPENCL_LIBRARIES})
install(TARGETS ch2_simple_convolution RUNTIME DESTINATION ${BIN_DIR}/ch2_simple_convolution)
install(FILES convolution.cl DESTINATION ${BIN_DIR}/ch2_simple_convolution)

