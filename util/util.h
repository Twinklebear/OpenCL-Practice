#ifndef UTIL_H
#define UTIL_H

#include <CL/cl.h>

/*
 * Read all contents of a file and return it, along with the size
 * read if desired. Caller must free the buffer later
 * returns NULL if fails
 */
char* read_file(const char *f_name, size_t *sz);
/*
 * Build an OpenCL program from the source for the context and device
 * and pass any desired compiler options
 * returns NULL on failure
 */
cl_program build_program(const char *src, cl_context context, cl_device_id device,
	const char *options);
/*
 * Check the error code for errors and log it and a message
 * returns 1 if an error was logged
 */
int check_cl_err(cl_int err, const char *msg);

#endif

