#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "util.h"
#include "cl_program_dir.h"

#define IN_DIM 8
#define MASK_DIM 3
#define OUT_DIM 6

//Select the first available OpenCL platform and make a context on it
cl_context get_first_platform();
//Select the first platform with the desired device type
cl_context get_platform(cl_device_type type);
//OpenCL callback for reporting errors in the context
void CL_CALLBACK cl_err_callback(const char *err_info, const void *priv_info, size_t cb, void *user);
//Select the first available device and set it and its command queue up
cl_command_queue get_first_device(cl_context context, cl_device_id *device);

int main(int argc, char **argv){
	cl_context context = get_platform(CL_DEVICE_TYPE_GPU);
	cl_device_id device = 0;
	cl_command_queue queue = get_first_device(context, &device);
	char *prog_src = read_file(CL_PROGRAM("convolution.cl"), NULL);
	cl_program program = build_program(prog_src, context, device, NULL);
	free(prog_src);
	cl_int err = CL_SUCCESS;
	cl_kernel kernel = clCreateKernel(program, "convolve", &err);
	check_cl_err(err, "failed to create kernel");

	//Setup our input signal and mask
	cl_uint in_signal[IN_DIM][IN_DIM] = {
		{ 3, 1, 1, 4, 8, 2, 1, 3 },
		{ 4, 2, 1, 1, 2, 1, 2, 3 },
		{ 4, 4, 4, 4, 3, 2, 2, 2 },
		{ 9, 8, 3, 8, 9, 0, 0, 0 },
		{ 9, 3, 3, 9, 0, 0, 0, 0 },
		{ 0, 9, 0, 8, 0, 0, 0, 0 },
		{ 3, 0, 8, 8, 9, 4, 4, 4 },
		{ 5, 9, 8, 1, 8, 1, 1, 1 }
	};
	cl_uint mask[MASK_DIM][MASK_DIM] = {
		{ 1, 1, 1 },
		{ 1, 0, 1 },
		{ 1, 1, 1 }
	};
	//0 is input, 1 is mask, 2 is output
	cl_mem mem_objs[3];
	mem_objs[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_uint) * IN_DIM * IN_DIM, in_signal, &err);
	mem_objs[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_uint) * MASK_DIM * MASK_DIM, mask, &err);
	mem_objs[2] = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
		sizeof(cl_uint) * OUT_DIM * OUT_DIM, NULL, &err);
	check_cl_err(err, "failed to create buffers");

	for (int i = 0; i < 3; ++i){
		err = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem_objs[i]);
		check_cl_err(err, "failed to set kernel argument");
	}
	size_t in_dim = IN_DIM, mask_dim = MASK_DIM;
	err = clSetKernelArg(kernel, 3, sizeof(unsigned), &in_dim);
	err = clSetKernelArg(kernel, 4, sizeof(unsigned), &mask_dim);
	check_cl_err(err, "failed to set kernel argument");

	size_t global_size[2] = { OUT_DIM, OUT_DIM };
	size_t local_size[2] = { 2, 2 };
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0,
		NULL, NULL);
	check_cl_err(err, "failed to enqueue ND range kernel");
	
	cl_uint* out = clEnqueueMapBuffer(queue, mem_objs[2], CL_TRUE, CL_MAP_READ, 0,
		sizeof(cl_uint) * OUT_DIM * OUT_DIM, 0, NULL, NULL, &err);
	check_cl_err(err, "failed to map result");

	printf("Result:\n");
	for (int i = 0; i < OUT_DIM; ++i){
		for (int j = 0; j < OUT_DIM; ++j){
			printf("%d ", out[i * OUT_DIM + j]);
		}
		printf("\n");
	}
	printf("\n");
	clEnqueueUnmapMemObject(queue, mem_objs[2], out, 0, 0, NULL);

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
cl_context get_platform(cl_device_type type){
	cl_uint num_platforms;
	cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
	cl_platform_id *platforms = malloc(sizeof(cl_platform_id) * num_platforms);
	err = clGetPlatformIDs(num_platforms, platforms, NULL);
	if (check_cl_err(err, "Failed to find platforms") || num_platforms < 1){
		return NULL;
	}
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, 0, 0
	};
	cl_context context = NULL;
	for (size_t i = 0; i < num_platforms; ++i){
		properties[1] = (cl_context_properties)platforms[i];
		context = clCreateContextFromType(properties, type, cl_err_callback, NULL, &err);
		if (err == CL_SUCCESS){
			char name[64];
			clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 64, name, NULL);
			printf("Selected platform: %s\n", name);
			break;
		}
	}
	free(platforms);
	return context;
}
void CL_CALLBACK cl_err_callback(const char *err_info, const void *priv_info, size_t cb, void *user){
	printf("OpenCL context error: %s\n", err_info);
	exit(EXIT_FAILURE);
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
			char name[64];
			clGetDeviceInfo(*device, CL_DEVICE_NAME, 64, name, NULL);
			printf("Selected device: %s\n", name);
			free(devices);
			return queue;
		}
	}
	fprintf(stderr, "Failed to create a command queue for any device\n");
	free(devices);
	return NULL;
}

