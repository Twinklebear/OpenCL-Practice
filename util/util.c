#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "util.h"

char* read_file(const char *f_name, size_t *sz){
	FILE *fp = fopen(f_name, "rb");
	if (!fp){
		fprintf(stderr, "read_file error: failed to open file: %s\n", f_name);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (sz){
		*sz = size;
	}
	char *content = malloc(sizeof(char) * size + sizeof(char));
	if (!content){
		fprintf(stderr, "read_file error: buffer allocation failed\n");
		return NULL;
	}
	if (fread(content, sizeof(char), size, fp) != size){
		fprintf(stderr, "read_file error: short read\n");
		free(content);
		return NULL;
	}
	content[size] = '\0';
	return content;
}
cl_program build_program(const char *src, cl_context context, cl_device_id device,
	const char *options)
{
	cl_int err;
	cl_program program = clCreateProgramWithSource(context, 1, &src, NULL, &err);
	if (check_cl_err(err, "Failed to create program from source")){
		return NULL;
	}
	err = clBuildProgram(program, 1, &device, options, NULL, NULL);
	if (check_cl_err(err, "Failed to build program for device")){
		size_t sz;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &sz);
		char *log = malloc(sizeof(char) * sz + 1);
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sz, log, NULL);
		log[sz] = '\0';
		fprintf(stderr, "BUILD LOG: %s\n", log);
		free(log);
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}
int check_cl_err(cl_int err, const char *msg){
	if (err == CL_SUCCESS){
		return 0;
	}
	fprintf(stderr, "--------\nMessage: %s\nError: ", msg);
	switch (err){
		case CL_DEVICE_NOT_FOUND:
			fprintf(stderr, "CL_DEVICE_NOT_FOUND\n");
			break;
		case CL_DEVICE_NOT_AVAILABLE:
			fprintf(stderr, "CL_DEVICE_NOT_AVAILABLE\n");
			break;
		case CL_COMPILER_NOT_AVAILABLE:
			fprintf(stderr, "CL_COMPILER_NOT_AVAILABLE\n");
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			fprintf(stderr, "CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
			break;
		case CL_OUT_OF_RESOURCES:
			fprintf(stderr, "CL_OUT_OF_RESOURCES\n");
			break;
		case CL_OUT_OF_HOST_MEMORY:
			fprintf(stderr, "CL_OUT_OF_HOST_MEMORY\n");
			break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:
			fprintf(stderr, "CL_PROFILING_INFO_NOT_AVAILABLE\n");
			break;
		case CL_MEM_COPY_OVERLAP:
			fprintf(stderr, "CL_MEM_COPY_OVERLAP\n");
			break;
		case CL_IMAGE_FORMAT_MISMATCH:
			fprintf(stderr, "CL_IMAGE_FORMAT_MISMATCH\n");
			break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			fprintf(stderr, "CL_IMAGE_FORMAT_NOT_SUPPORTED\n");
			break;
		case CL_BUILD_PROGRAM_FAILURE:
			fprintf(stderr, "CL_BUILD_PROGRAM_FAILURE\n");
			break;
		case CL_MAP_FAILURE:
			fprintf(stderr, "CL_MAP_FAILURE\n");
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			fprintf(stderr, "CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
			break;
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			fprintf(stderr, "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
			break;
		case CL_INVALID_VALUE:
			fprintf(stderr, "CL_INVALID_VALUE\n");
			break;
		case CL_INVALID_DEVICE_TYPE:
			fprintf(stderr, "CL_INVALID_DEVICE_TYPE\n");
			break;
		case CL_INVALID_PLATFORM:
			fprintf(stderr, "CL_INVALID_PLATFORM\n");
			break;
		case CL_INVALID_DEVICE:
			fprintf(stderr, "CL_INVALID_DEVICE\n");
			break;
		case CL_INVALID_CONTEXT:
			fprintf(stderr, "CL_INVALID_CONTEXT\n");
			break;
		case CL_INVALID_QUEUE_PROPERTIES:
			fprintf(stderr, "CL_INVALID_QUEUE_PROPERTIES\n");
			break;
		case CL_INVALID_COMMAND_QUEUE:
			fprintf(stderr, "CL_INVALID_COMMAND_QUEUE\n");
			break;
		case CL_INVALID_HOST_PTR:
			fprintf(stderr, "CL_INVALID_HOST_PTR\n");
			break;
		case CL_INVALID_MEM_OBJECT:
			fprintf(stderr, "CL_INVALID_MEM_OBJECT\n");
			break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			fprintf(stderr, "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n");
			break;
		case CL_INVALID_IMAGE_SIZE:
			fprintf(stderr, "CL_INVALID_IMAGE_SIZE\n");
			break;
		case CL_INVALID_SAMPLER:
			fprintf(stderr, "CL_INVALID_SAMPLER\n");
			break;
		case CL_INVALID_BINARY:
			fprintf(stderr, "CL_INVALID_BINARY\n");
			break;
		case CL_INVALID_BUILD_OPTIONS:
			fprintf(stderr, "CL_INVALID_BUILD_OPTIONS\n");
			break;
		case CL_INVALID_PROGRAM:
			fprintf(stderr, "CL_INVALID_PROGRAM\n");
			break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			fprintf(stderr, "CL_INVALID_PROGRAM_EXECUTABLE\n");
			break;
		case CL_INVALID_KERNEL_NAME:
			fprintf(stderr, "CL_INVALID_KERNEL_NAME\n");
			break;
		case CL_INVALID_KERNEL_DEFINITION:
			fprintf(stderr, "CL_INVALID_KERNEL_DEFINITION\n");
			break;
		case CL_INVALID_KERNEL:
			fprintf(stderr, "CL_INVALID_KERNEL\n");
			break;
		case CL_INVALID_ARG_INDEX:
			fprintf(stderr, "CL_INVALID_ARG_INDEX\n");
			break;
		case CL_INVALID_ARG_VALUE:
			fprintf(stderr, "CL_INVALID_ARG_VALUE\n");
			break;
		case CL_INVALID_ARG_SIZE:
			fprintf(stderr, "CL_INVALID_ARG_SIZE\n");
			break;
		case CL_INVALID_KERNEL_ARGS:
			fprintf(stderr, "CL_INVALID_KERNEL_ARGS\n");
			break;
		case CL_INVALID_WORK_DIMENSION:
			fprintf(stderr, "CL_INVALID_WORK_DIMENSION\n");
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			fprintf(stderr, "CL_INVALID_WORK_GROUP_SIZE\n");
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			fprintf(stderr, "CL_INVALID_WORK_ITEM_SIZE\n");
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			fprintf(stderr, "CL_INVALID_GLOBAL_OFFSET\n");
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			fprintf(stderr, "CL_INVALID_EVENT_WAIT_LIST\n");
			break;
		case CL_INVALID_EVENT:
			fprintf(stderr, "CL_INVALID_EVENT\n");
			break;
		case CL_INVALID_OPERATION:
			fprintf(stderr, "CL_INVALID_OPERATION\n");
			break;
		case CL_INVALID_GL_OBJECT:
			fprintf(stderr, "CL_INVALID_GL_OBJECT\n");
			break;
		case CL_INVALID_BUFFER_SIZE:
			fprintf(stderr, "CL_INVALID_BUFFER_SIZE\n");
			break;
		case CL_INVALID_MIP_LEVEL:
			fprintf(stderr, "CL_INVALID_MIP_LEVEL\n");
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			fprintf(stderr, "CL_INVALID_GLOBAL_WORK_SIZE\n");
			break;
		case CL_INVALID_PROPERTY:
			fprintf(stderr, "CL_INVALID_PROPERTY\n");
			break;
#ifdef CL_VERSION_1_2
		case CL_INVALID_IMAGE_DESCRIPTOR:
			fprintf(stderr, "CL_INVALID_IMAGE_DESCRIPTOR\n");
			break;
		case CL_INVALID_COMPILER_OPTIONS:
			fprintf(stderr, "CL_INVALID_COMPILER_OPTIONS\n");
			break;
		case CL_INVALID_LINKER_OPTIONS:
			fprintf(stderr, "CL_INVALID_LINKER_OPTIONS\n");
			break;
		case CL_INVALID_DEVICE_PARTITION_COUNT:
			fprintf(stderr, "CL_INVALID_DEVICE_PARTITION_COUNT\n");
			break;
#endif
		default:
			fprintf(stderr, "UNRECOGNIZED ERROR\n");
			break;
	}
	fprintf(stderr, "--------\n");
	return 1;
}

