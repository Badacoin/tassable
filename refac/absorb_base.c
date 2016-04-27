#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

#define FLAT_HEIGHT 5

int **init;
int **temp;

bool
absorb_base (int **table, int dim, int iterations)
{
    bool finished = true;

    for (int i = 0 ; i < dim - 1 ; i++) {
	for (int j = 0 ; j < dim - 1 ; j++) {
	    init[i][j] = table[i][j];
	}
    }
    
    for (int k = 0 ; k < iterations; k++) {
	for (int i = 1 ; i < dim - 1 ; i++) {
	    for (int j = 1 ; j < dim - 1 ; j++) {
		int middle = table[i][j] % 4;
		int left = table[i][j-1] / 4;
		int right = table[i][j+1] / 4;
		int up = table[i-1][j] / 4;
		int down = table[i+1][j] /4;
		temp[i][j] = middle + up + down + left + right;
	    }
	}
	
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j = 1 ; j < dim - 1 ; j++) {
		table[i][j] = temp[i][j];
	    }
	}
    }

    for (int i = 1 ; finished && i < dim - 1 ; i++) {	
	for (int j = 1 ; finished && j < dim - 1 ; j++) {
	    finished = finished && (table[i][j] == init[i][j]);
	}
    }
    
    return finished;
}

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool check = false;
    int dim = 512;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = absorb_base;
    temp = table_alloc(dim);
    init = table_alloc(dim);
    
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

    int **table = table_alloc(dim);
    if (tower_height != 0) {
	tower_init(table, tower_height, dim);
    }
    else {
	flat_init(table, FLAT_HEIGHT, dim);
    }

    if (graphical) {
	display_init (0, NULL, dim, table, get, func);
    }
    else {
	run(func, table, dim, iterations);
    }

    if (check) {
	int **control = table_alloc(dim);
	
	if (tower_height != 0) {
	    tower_init(control, tower_height, dim);
	}
	else {
	    flat_init(control, FLAT_HEIGHT, dim);
	}

	printf("\nProcessing control table.\n");
	run(naive, control, dim, iterations);
	printf("\n");
	compare(table, control, dim);
	table_free(control);
    }

    table_free(table);
    table_free(temp);
    table_free(init);
    return 0;
}
