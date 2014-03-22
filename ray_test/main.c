#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "util.h"
#include "cl_program_dir.h"

#define IMG_DIM 16

typedef struct sphere_t {
	cl_float3 center;
	float radius;
} sphere_t;

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
	char *prog_src = read_file(CL_PROGRAM("ray_test.cl"), NULL);
	cl_program program = build_program(prog_src, context, device, NULL);
	free(prog_src);
	cl_int err = CL_SUCCESS;
	cl_kernel kernel = clCreateKernel(program, "cast_rays", &err);
	check_cl_err(err, "failed to create kernel");

	cl_mem mem_ray_start = clCreateBuffer(context, CL_MEM_READ_ONLY,
		IMG_DIM * IMG_DIM * sizeof(cl_float3), NULL, &err);
	check_cl_err(err, "failed to create buffer");
	cl_float3 *ray_starts = clEnqueueMapBuffer(queue, mem_ray_start, CL_FALSE, CL_MAP_WRITE,
		0, IMG_DIM * IMG_DIM * sizeof(cl_float3), 0, NULL, NULL, &err);
	check_cl_err(err, "failed to map buffer");
	for (int i = 0; i < IMG_DIM; ++i){
		for (int j = 0; j < IMG_DIM; ++j){
			ray_starts[i * IMG_DIM + j].s[0] = i;
			ray_starts[i * IMG_DIM + j].s[1] = j;
			ray_starts[i * IMG_DIM + j].s[3] = 0;
		}
	}
	clEnqueueUnmapMemObject(queue, mem_ray_start, ray_starts, 0, 0, NULL);

	clReleaseMemObject(mem_ray_start);
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

