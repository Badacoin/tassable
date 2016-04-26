#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int
get (int i, int j, int **table)
{
    return table[j][i];		/* TODO: ??? */
}

void
flat_init (int **table, int height, int dim)
{
    for (int i = 1; i < dim - 1 ; i++) {
	for (int j = 1; j < dim - 1 ; j++) {
	    table[i][j] = height;
	}
    }
}

void
tower_init (int **table, int height, int dim)
{
    for (int i = 1; i < dim - 2 ; i++) {
	for (int j = 1; j < dim -2 ; j++) {
	    table[i][j] = 0;
	}
    }
    table[dim/2][dim/2] = height;
}

int **
table_alloc (int dim)
{
    int **table = malloc(dim * sizeof(int *));
    if (table == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }

    for (int i = 0 ; i < dim ; i++) {
	if ((table[i] = malloc(dim * sizeof(int))) == NULL) {
	    fputs("Error: out of memory!\n", stderr);
	    exit(EXIT_FAILURE);
	}
    }
    
    return table;
}

void
run (compute_func_t compute_func, int **table,
     int dim, unsigned iterations)
{
    bool finished = false;
    int computeTime = 0;
    unsigned call_counter = 0;
    while (!finished) {
	struct timeval t1,t2;
	gettimeofday (&t1,NULL);
	finished = compute_func(table, dim, iterations);
	gettimeofday (&t2,NULL);
	computeTime += TIME_DIFF(t1,t2);
	call_counter++;
    }

    printf("%d function calls\n", call_counter);
    printf("time per iteration : %.3f ms\n",
	   computeTime / (float) (1000 * call_counter * iterations));
    printf("total time : %.3f s\n",
	   computeTime / (float) (1000 * 1000));
}
