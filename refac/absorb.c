#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int dim = 512;
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
absorb (int iterations)
{
    bool finished = true;
    int **src, **dst, **temp;
    src = (parity % 2) ? dual : table;
    dst = (parity % 2) ? table : dual;

    for (int i = 0 ; i < dim - 1 ; i++) {
	for (int j = 0 ; j < dim - 1 ; j++) {
	    init[i][j] = src[i][j];
	}
    }
    
    for (int k = 0 ; k < iterations; k++) {
	for (int i = 1 ; i < dim - 1 ; i++) {
	    for (int j = 1 ; j < dim - 1 ; j++) {
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

    for (int i = 1 ; finished && i < dim - 1 ; i++) {	
	for (int j = 1 ; finished && j < dim - 1 ; j++) {
	    finished = finished && (src[i][j] == init[i][j]);
	}
    }
    
    return finished;
}

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool check = false;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = absorb;
    
    while ((optc = getopt(argc, argv, "N:t:i:gc")) != -1) {
	switch (optc) {
	    case 'N' :
		dim = strtol(optarg, NULL, 10);
		break;
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
		check = true;
		break;
	}
    }

    table = table_alloc(dim);
    dual = table_alloc(dim);
    init = table_alloc(dim);
    flat_init(dual, 0, dim);
    
    if (tower_height != 0) {
	tower_init(table, tower_height, dim);
    }
    else {
	flat_init(table, FLAT_HEIGHT, dim);
    }

    if (graphical) {
	display_init (0, NULL, dim, get, func);
    }
    else {
	run(func, iterations);
    }

    if (check) {
	int **control = table_alloc(dim);
	
	if (tower_height != 0) {
	    tower_init(control, tower_height, dim);
	}
	else {
	    flat_init(control, FLAT_HEIGHT, dim);
	}

	process(control, dim);
	compare(table, control, dim);
	table_free(control);
    }

    table_free(table);
    table_free(dual);
    table_free(init);
    return 0;
}
