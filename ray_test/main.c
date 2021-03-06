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
#define N_OBJS 3

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

	cl_mem mem_spheres = clCreateBuffer(context, CL_MEM_READ_ONLY,
		N_OBJS * sizeof(sphere_t), NULL, &err);
	check_cl_err(err, "failed to create buffer");

	cl_mem mem_img = clCreateBuffer(context, CL_MEM_WRITE_ONLY, IMG_DIM * IMG_DIM * sizeof(cl_char),
		NULL, &err);
	check_cl_err(err, "failed to create buffer");

	cl_float3 *ray_starts = clEnqueueMapBuffer(queue, mem_ray_start, CL_FALSE, CL_MAP_WRITE,
		0, IMG_DIM * IMG_DIM * sizeof(cl_float3), 0, NULL, NULL, &err);
	check_cl_err(err, "failed to create buffer");

	sphere_t *spheres = clEnqueueMapBuffer(queue, mem_spheres, CL_TRUE, CL_MAP_WRITE,
		0, N_OBJS * sizeof(sphere_t), 0, NULL, NULL, &err);
	check_cl_err(err, "failed to map buffer");
	spheres[0] = (sphere_t){
		.center = {{ IMG_DIM / 2 - 1, IMG_DIM / 2 - 1, 3 }},
		.radius = 2.9
	};
	spheres[1] = (sphere_t){
		.center = {{ 3, 2, 2 }},
		.radius = 1.2
	};
	spheres[2] = (sphere_t){
		.center = {{ IMG_DIM - 2, IMG_DIM - 4, 5 }},
		.radius = 3.5
	};

	cl_char background = ' ';
	err = clEnqueueFillBuffer(queue, mem_img, &background, sizeof(cl_char), 0,
		IMG_DIM * IMG_DIM * sizeof(cl_char), 0, 0, NULL);
	check_cl_err(err, "failed to fill buffer");

	for (int i = 0; i < IMG_DIM; ++i){
		for (int j = 0; j < IMG_DIM; ++j){
			ray_starts[i * IMG_DIM + j].s[0] = i;
			ray_starts[i * IMG_DIM + j].s[1] = j;
			ray_starts[i * IMG_DIM + j].s[3] = 0;
		}
	}
	clEnqueueUnmapMemObject(queue, mem_ray_start, ray_starts, 0, 0, NULL);
	clEnqueueUnmapMemObject(queue, mem_spheres, spheres, 0, 0, NULL);

	cl_uint n_objs = N_OBJS;
	cl_uint2 dim = {{ IMG_DIM, IMG_DIM }};
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_ray_start);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_spheres);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &n_objs);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &mem_img);
	err |= clSetKernelArg(kernel, 4, sizeof(cl_uint2), &dim);
	check_cl_err(err, "failed to set one or more kernel args");

	size_t global_size[2] = { IMG_DIM, IMG_DIM };
	size_t local_size[2] = { 2, 2 };
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size,
		local_size, 0, NULL, NULL);
	check_cl_err(err, "failed to run kernel");

	cl_char *img = clEnqueueMapBuffer(queue, mem_img, CL_TRUE, CL_MAP_READ,
		0, IMG_DIM * IMG_DIM * sizeof(cl_char), 0, NULL, NULL, &err);
	check_cl_err(err, "failed to map img buffer");

	for (int i = 0; i < IMG_DIM; ++i){
		printf("| ");
		for (int j = 0; j < IMG_DIM; ++j){
			printf("%c", img[i * IMG_DIM + j]);
		}
		printf(" |\n");
	}
	clEnqueueUnmapMemObject(queue, mem_img, img, 0, 0, NULL);
	clFinish(queue);

	clReleaseMemObject(mem_ray_start);
	clReleaseMemObject(mem_spheres);
	clReleaseMemObject(mem_img);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	return 0;
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

