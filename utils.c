#include "utils.h"

#include <stdlib.h>

void
flat_init (int **table, int height, int dim)
{
    for (int i = 1; i < dim - 1 ; i++) {
	for (int j = 1; j < dim - 1 ; j++) {
	    table[i][j] = height;
	}
    }
    for (int i = 0 ; i < dim ; i++) {
	table[0][i] = 0;
	table[dim-1][i] = 0;
	table[i][0] = 0;
	table[i][dim-1] = 0;
    }
}

void
tower_init (int **table, int height, int dim)
{
    for (int i = 0 ; i < dim ; i++) {
	for (int j = 0; j < dim ; j++) {
	    table[i][j] = 0;
	}
    }
    table[dim/2][dim/2] = height;
}

int **
table_alloc (int dim)
{
    int *linear_table = malloc(dim * dim * sizeof(int));
    if (linear_table == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }

    int **table = malloc(dim * sizeof(int *));
    if (table == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    
    int offset = 0;
    for (int i = 0 ; i < dim ; i++) {
        table[i] = &linear_table[offset];
	offset += dim;
    }
    
    return table;
}

void
table_free (int **table)
{
    if (table == NULL) {
    }
    else {      
	free(table[0]);
	free(table);
    }
}

void
run (compute_func_t compute_func, unsigned iterations)
{
    bool finished = false;
    int computeTime = 0;
    unsigned call_counter = 0;
    while (!finished) {
	struct timeval t1,t2;
	gettimeofday (&t1,NULL);
	finished = compute_func(iterations);
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

void
process (int **control, int dim)
{
    bool finished = false;

    while (!finished) {
	finished = true;
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j= 1 ; j < dim - 1 ; j++) {
		if (control[i][j] >= 4) {
		    finished = false;
		    int mod4 = control[i][j] % 4;      
		    int div4 = control[i][j] / 4;
		    control[i][j] = mod4;   
		    control[i-1][j] += div4;   
		    control[i+1][j] += div4;   
		    control[i][j-1] += div4;   
		    control[i][j+1] += div4;   
		}
	    }
	}
    }
}

void
compare (int **table, int **control, int dim)
{
    bool equal = true;
    for (int i = 1 ; i < dim - 1 ; i++) {
	for (int j = 1 ; j < dim - 1 ; j++) {
	    equal = equal && (table[i][j] == control[i][j]);
	}
    }
    
    if (equal) {
	printf("Check successful!\n");
    }
    else {
	printf("There was a mistake.\n");
    }
}
