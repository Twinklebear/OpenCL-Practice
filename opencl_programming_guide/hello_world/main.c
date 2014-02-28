#include <stdio.h>
#include <CL/cl.h>
#include "util.h"

//Select the first available OpenCL platform and make a context on it
cl_context get_first_platform();

int main(int argc, char **argv){
	cl_context context = get_first_platform();
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

