#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int dim = 512;
int **table;

int
get (int i, int j)
{
    return table[i][j];
}

bool
naive (int iterations)
{
    bool finished = true;
    
    for (int k = 0 ; k < iterations ; k++) {
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j= 1 ; j < dim - 1 ; j++) {
		if (table[i][j] >= 4) {
		    finished = false;
		    int mod4 = table[i][j] % 4;      
		    int div4 = table[i][j] / 4;
		    table[i][j] = mod4;   
		    table[i-1][j] += div4;   
		    table[i+1][j] += div4;   
		    table[i][j-1] += div4;   
		    table[i][j+1] += div4;   
		}
	    }
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
    compute_func_t func = naive;
    
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
    return 0;
}
