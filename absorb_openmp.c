#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int **table;
int **dual;
int **init;
int parity = 0;

int
get (int i, int j)
{
    return (parity % 2) ? dual[i][j] : table[i][j];
}

bool
absorb_openmp (int iterations)
{
    bool finished = true;
    int **src, **dst, **temp;
    src = (parity % 2) ? dual : table;
    dst = (parity % 2) ? table : dual;

    #pragma omp parallel for
    for (int i = 0 ; i < DIM - 1 ; i++) {
	for (int j = 0 ; j < DIM - 1 ; j++) {
	    init[i][j] = src[i][j];
	}
    }
    
    for (int k = 0 ; k < iterations; k++) {
	#pragma omp parallel for
	for (int i = 1 ; i < DIM - 1 ; i++) {
	    for (int j = 1 ; j < DIM - 1 ; j++) {
		int left = src[i][j-1] / 4;
		int middle = src[i][j] % 4;
		int right = src[i][j+1] / 4;
		int up = src[i-1][j] / 4;
		int down = src[i+1][j] /4;
		dst[i][j] = left + middle + right + up + down;
	    }
	}
	
	temp = dst;
	dst = src;
	src = temp;
	parity++;
    }

    for (int i = 1 ; finished && i < DIM - 1 ; i++) {	
	for (int j = 1 ; finished && j < DIM - 1 ; j++) {
	    finished = finished && (src[i][j] == init[i][j]);
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
    compute_func_t func = absorb_openmp;
    
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

    table = table_alloc(DIM);
    dual = table_alloc(DIM);
    init = table_alloc(DIM);
    flat_init(dual, 0, DIM);
    
    if (tower_height != 0) {
	tower_init(table, tower_height, DIM);
    }
    else {
	flat_init(table, FLAT_HEIGHT, DIM);
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
	    flat_init(control, FLAT_HEIGHT, DIM);
	}

	process(control, DIM);
	compare(table, control, DIM);
	table_free(control);
    }

    table_free(table);
    table_free(dual);
    table_free(init);
    return 0;
}
