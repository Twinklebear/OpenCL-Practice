#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "util.h"
#include "cl_program_dir.h"

//Select the first available OpenCL platform and make a context on it
cl_context get_first_platform();
//Select the first available device and set it and its command queue up
cl_command_queue get_first_device(cl_context context, cl_device_id *device);

int main(int argc, char **argv){
	cl_context context = get_first_platform();
	cl_device_id device = 0;
	cl_command_queue queue = get_first_device(context, &device);
	char *prog_src = read_file(CL_PROGRAM("hello_world.cl"), NULL);
	cl_program program = build_program(prog_src, context, device, NULL);
	free(prog_src);

	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	return 0;
}
cl_context get_first_platform(){
	cl_uint num_platforms;
	cl_platform_id platform;
	cl_int err = clGetPlatformIDs(1, &platform, &num_platforms);
	if (check_cl_err(err, "Failed to find a platform") || num_platforms < 1){
		return NULL;
	}
	char name[64];
	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 64, name, NULL);
	//This error probably shouldn't happen, but check anyway
	check_cl_err(err, "Failed to get platform name");
	printf("Selecting platform: %s\n", name);

	//Try to get a GPU context on the platform
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
	};
	cl_context context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU,
		NULL, NULL, &err);
	if (check_cl_err(err, "Failed to create GPU context, retrying CPU")){
		context = clCreateContextFromType(properties, CL_DEVICE_TYPE_CPU,
			NULL, NULL, &err);
		if (check_cl_err(err, "Failed to create a GPU or CPU context")){
			return NULL;
		}
	}
	return context;
}
cl_command_queue get_first_device(cl_context context, cl_device_id *device){
	size_t num_devices;
	cl_int err = clGetContextInfo(context, CL_CONTEXT_NUM_DEVICES,
		sizeof(num_devices), &num_devices, NULL);
	if (check_cl_err(err, "Failed to get number of devices")){
		return NULL;
	}
	if (num_devices < 1){
		fprintf(stderr, "No devices available\n");
		return NULL;
	}

	cl_device_id *devices = malloc(sizeof(cl_device_id) * num_devices);
	err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * num_devices,
		devices, NULL);
	if (check_cl_err(err, "Failed to get devices for context")){
		free(devices);
		return NULL;
	}

	//Create a command queue on the first device we can and use that device
	for (size_t i = 0; i < num_devices; ++i){
		cl_command_queue queue = clCreateCommandQueue(context, devices[i], 0, &err);
		if (err == CL_SUCCESS){
			*device = devices[i];
			free(devices);
			return queue;
		}
	}
	fprintf(stderr, "Failed to create a command queue for any device\n");
	free(devices);
	return NULL;
}

