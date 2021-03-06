#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "util.h"
#include "cl_program_dir.h"

#define ARRAY_SIZE 16

//Select the first available OpenCL platform and make a context on it
cl_context get_first_platform();
//OpenCL callback for reporting errors in the context
void CL_CALLBACK cl_err_callback(const char *err_info, const void *priv_info, size_t cb, void *user);
//Select the first available device and set it and its command queue up
cl_command_queue get_first_device(cl_context context, cl_device_id *device);

int main(int argc, char **argv){
	cl_context context = get_first_platform();
	cl_device_id device = 0;
	cl_command_queue queue = get_first_device(context, &device);
	char *prog_src = read_file(CL_PROGRAM("hello_world.cl"), NULL);
	cl_program program = build_program(prog_src, context, device, NULL);
	free(prog_src);
	cl_int err = CL_SUCCESS;
	cl_kernel kernel = clCreateKernel(program, "hello_world", &err);
	check_cl_err(err, "failed to create kernel");

	cl_mem mem_objs[3];
	for (int i = 0; i < 2; ++i){
		mem_objs[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, ARRAY_SIZE * sizeof(cl_float), NULL, &err);
		check_cl_err(err, "failed to create buffer");
	}
	mem_objs[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, ARRAY_SIZE * sizeof(cl_float), NULL, &err);
	check_cl_err(err, "failed to create buffer");

	cl_float* mem[3];
	for (int i = 0; i < 3; ++i){
		mem[i] = clEnqueueMapBuffer(queue, mem_objs[i], CL_FALSE, CL_MAP_WRITE, 0,
			ARRAY_SIZE * sizeof(cl_float), 0, NULL, NULL, &err);
		check_cl_err(err, "failed to map buffer");
	}
	clFinish(queue);
	for (int i = 0; i < ARRAY_SIZE; ++i){
		mem[0][i] = i;
		mem[1][i] = i;
		mem[2][i] = 0;
	}
	for (int i = 0; i < 3; ++i){
		err = clEnqueueUnmapMemObject(queue, mem_objs[i], mem[i], 0, NULL, NULL);
		check_cl_err(err, "failed to unmap mem object");
	}

	size_t global_size[1] = { ARRAY_SIZE };
	size_t local_size[1] = { 8 };
	for (int i = 0; i < 3; ++i){
		err = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem_objs[i]);
		check_cl_err(err, "failed to set kernel argument");
	}
	err = clSetKernelArg(kernel, 3, sizeof(size_t), &global_size[0]);
	check_cl_err(err, "failed to set kernel argument");

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_size, local_size, 0, NULL, NULL);
	check_cl_err(err, "failed to enqueue ND range kernel");
	
	mem[2] = clEnqueueMapBuffer(queue, mem_objs[2], CL_TRUE, CL_MAP_READ, 0,
		ARRAY_SIZE * sizeof(cl_float), 0, NULL, NULL, &err);
	check_cl_err(err, "failed to map result");

	printf("Result: ");
	for (int i = 0; i < ARRAY_SIZE; ++i){
		printf("%.2f, ", mem[2][i]);
	}
	printf("\n");
	clEnqueueUnmapMemObject(queue, mem_objs[2], mem[2], 0, 0, NULL);

	for (int i = 0; i < 3; ++i){
		clReleaseMemObject(mem_objs[i]);
	}
	clReleaseKernel(kernel);
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
		cl_err_callback, NULL, &err);
	if (check_cl_err(err, "Failed to create GPU context, retrying CPU")){
		context = clCreateContextFromType(properties, CL_DEVICE_TYPE_CPU,
			NULL, NULL, &err);
		if (check_cl_err(err, "Failed to create a GPU or CPU context")){
			return NULL;
		}
	}
	return context;
}
void CL_CALLBACK cl_err_callback(const char *err_info, const void *priv_info, size_t cb, void *user){
	printf("OpenCL context error: %s\n", err_info);
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

