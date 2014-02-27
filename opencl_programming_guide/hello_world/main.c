#include <stdio.h>
#include <CL/cl.h>
#include "util.h"

int main(int argc, char **argv){
	cl_uint num_platforms = 0;
	cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
	if (check_cl_err(err, "getting number of platforms")){
		return 1;
	}
	printf("Number of platforms: %d\n", num_platforms);

	return 0;
}

