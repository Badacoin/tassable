#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <sys/time.h>

#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

typedef int (*get_func_t) (int i, int j, int **table);
typedef bool (*compute_func_t)
    (int **table, int dim, int iterations);

int get (int i, int j, int **table);
void flat_init (int **table, int height, int dim);
void tower_init (int **table, int height, int dim);
int **table_alloc (int dim);
void run (compute_func_t compute_func, int **table,
	  int dim, unsigned iterations);

#endif
