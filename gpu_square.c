#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#define SIZE DIM - 2
#define TILE 16
#define KERNEL "gpu_square"
#define KERNEL_FILE "gpu_square_kernel.cl"

int **matrix_table;
int **init;
int *table;

size_t global[2] = {SIZE - TILE, SIZE - TILE};
size_t local[2]  = {TILE, TILE}; 

cl_context context;
cl_kernel kernel;
cl_mem table_buffer;
cl_mem dual_buffer;
cl_command_queue queue;

int
get (int i, int j)
{
    return table[i * DIM + j];
}

static void alloc_buffers_and_user_data(cl_context context)
{
    cl_int err;
    matrix_table = table_alloc(DIM);
    table = matrix_table[0];
    init = table_alloc(DIM);

    table_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(int) * DIM * DIM, NULL, NULL);
    if (!table_buffer) {
	printf("Failed to allocate input buffer A");
    }

    dual_buffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
				  sizeof(int) * DIM * DIM, NULL, NULL);
    if (!dual_buffer) {
	printf("Failed to allocate input buffer A");
    }

    int zero = 0;
    err = clEnqueueFillBuffer(queue, dual_buffer, &zero, sizeof(int),
			      0, sizeof(int) * DIM * DIM, 0, NULL, NULL);
    check(err, "Failed to initialize buffer");
    
}

static void free_buffers_and_user_data(void)
{
    table_free(matrix_table);
    table_free(init);
    clReleaseMemObject(table_buffer);
    clReleaseMemObject(dual_buffer);
}

static void send_input(void)
{
    cl_int err;
    err = clEnqueueWriteBuffer(queue, table_buffer, CL_TRUE, 0,
			       sizeof(int) * DIM * DIM,
			       table, 0, NULL, NULL);
    check(err, "Failed to transfer input");
}

static void retrieve_output(cl_mem src)
{
    cl_int err;
    err = clEnqueueReadBuffer(queue, src, CL_TRUE, 0,
			      sizeof(int) * DIM * DIM,
			      table, 0, NULL, NULL);  
    check(err, "Failed to transfer output");
}

bool
gpu_square (int iterations)
{
    bool finished = true;
    send_input();
    cl_mem src;
    cl_mem dst;
    cl_mem temp;
    src = table_buffer;
    dst = dual_buffer;
    cl_int err;

    #pragma omp parallel for
    for (int i = 0 ; i < DIM - 1 ; i++) {
	for (int j = 0 ; j < DIM - 1 ; j++) {
	    init[i][j] = matrix_table[i][j];
	}
    }
    
    for (int k = 0 ; k < iterations; k++) {
	err = 0;
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dst);
	check(err, "Failed to set kernel arguments");
	
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL,
				     global, local, 0, NULL, NULL);
	check(err, "Failed to execute kernel");
	clFinish(queue);
	
	temp = dst;
	dst = src;
	src = temp;
    }

    retrieve_output(src);
    #pragma omp parallel reduction(&&:finished)
    for (int i = 1 ; finished && i < DIM - 1 ; i++) {	
	for (int j = 1 ; finished && j < DIM - 1 ; j++) {
	    finished = finished && (matrix_table[i][j] == init[i][j]);
	}
    }
    
    return finished;
}

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool validation = false;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = gpu_square;
    
    while ((optc = getopt(argc, argv, "t:i:gc")) != -1) {
	switch (optc) {
	    case 't' :
	        tower_height = strtol(optarg, NULL, 10);
	        break;
	    case 'i' :
	        iterations = strtol(optarg, NULL, 10);
	        break;
	    case 'g' :
		graphical = true;
		break;
	    case 'c' :
		validation = true;
		break;
	}
    }   

    cl_platform_id pf[MAX_PLATFORMS];
    cl_uint nb_platforms = 0;
    cl_int err;
    cl_device_type device_type = CL_DEVICE_TYPE_ALL;
    err = clGetPlatformIDs(2, pf, &nb_platforms);
    check(err, "Failed to get platform IDs");
    cl_uint gpu_platform = MAX_PLATFORMS;
    cl_uint gpu_device = MAX_DEVICES;
    cl_device_id devices[MAX_DEVICES];
    cl_uint nb_devices = 0;
    cl_program program;
    

    for (cl_uint p = 0 ; p < nb_platforms ; p++) {
	err = clGetDeviceIDs(pf[p], device_type, MAX_DEVICES,
			     devices, &nb_devices);
	check(err, "Failed to get Device Info");
	for (cl_uint d = 0 ; d < nb_devices ; d++) {
	    cl_device_type dtype;
	    err = clGetDeviceInfo(devices[d], CL_DEVICE_TYPE,
				  sizeof(cl_device_type), &dtype, NULL);
	    check(err, "Failed to get Device Info");
	    if (dtype == CL_DEVICE_TYPE_GPU) {
		gpu_platform = p;
		gpu_device = d;
	    }
	}
    }

    if (gpu_platform == MAX_PLATFORMS) {
	printf("Failed to find a GPU\n");
	exit(EXIT_FAILURE);	
    }

    err = clGetDeviceIDs(pf[gpu_platform], device_type, MAX_DEVICES,
			 devices, &nb_devices);
    check(err, "Failed to get Device Info");

    context = clCreateContext (0, nb_devices, devices, NULL, NULL, &err);
    check(err, "Failed to create compute context");

    const char	*opencl_prog;

    char flags[1024];
    sprintf(flags, "-DSIZE=%d -DTILE=%d -DDIM=%d", SIZE, TILE, DIM);

    opencl_prog = file_load(KERNEL_FILE);
    program = clCreateProgramWithSource(context, 1, &opencl_prog,
					NULL, &err);
    check(err, "Failed to create program");
    compile(program, devices[gpu_device], flags);

    kernel = clCreateKernel(program, KERNEL, &err);
    check(err, "Failed to create kernel");

    queue = clCreateCommandQueue(context, devices[gpu_device],
				 CL_QUEUE_PROFILING_ENABLE, &err);
    check(err,"Failed to create command queue");

    alloc_buffers_and_user_data(context);
    if (tower_height != 0) {
	tower_init(matrix_table, tower_height, DIM);
    }
    else {
	flat_init_center(matrix_table, FLAT_HEIGHT, DIM, TILE / 2);
    }
    
    if (graphical) {
	display_init (0, NULL, DIM, get, func);
    }
    else {
	run(func, iterations);
    }

    if (validation) {
	int **control = table_alloc(DIM);
	
	if (tower_height != 0) {
	    tower_init(control, tower_height, DIM);
	}
	else {
	    flat_init_center(control, FLAT_HEIGHT, DIM, TILE / 2);
	}

	process(control, DIM);
	compare(matrix_table, control, DIM);
	table_free(control);
    }

    free_buffers_and_user_data();
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseCommandQueue(queue);
    return 0;
}
