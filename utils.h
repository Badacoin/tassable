#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <CL/opencl.h>

#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

#define DIM 514
#define FLAT_HEIGHT 5
#define MAX_PLATFORMS (unsigned) 3
#define MAX_DEVICES (unsigned) 5

typedef int (*get_func_t) (int i, int j);
typedef bool (*compute_func_t) (int iterations);

void check (cl_int err, char *message);
void flat_init (int **table, int height, int dim);
void tower_init (int **table, int height, int dim);
int **table_alloc (int dim);
void table_free (int **table);
void run (compute_func_t compute_func, unsigned iterations);
void process (int **control, int dim);
void compare (int **table, int **control, int dim);

size_t file_size(const char *filename);
char *file_load(const char *filename);

void compile (cl_program program,
	      cl_device_id device, char *flags);

#endif
