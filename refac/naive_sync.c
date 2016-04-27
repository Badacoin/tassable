#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

#define FLAT_HEIGHT 5

bool
naive_sync (int **table, int dim, int iterations)
{
    bool finished = true;
    int **temp = table_alloc(dim);

    for (int k = 0 ; k < iterations ; k++) {
	
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j = 1 ; j< dim - 1 ; j++) {
		temp[i][j] = 0;
	    }
	}
	
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j = 1 ; j < dim - 1 ; j++) {
		if (table[i][j] >= 4) {
		    finished = false;
		    int mod4 = table[i][j] % 4;      
		    int div4 = table[i][j] / 4;
		    temp[i][j] -= table[i][j] - mod4;   
		    temp[i-1][j] += div4;   
		    temp[i+1][j] += div4;   
		    temp[i][j-1] += div4;   
		    temp[i][j+1] += div4;   
		}
	    }
	}
	
	for (int i = 1 ; i < dim - 1 ; i++) {	
	    for (int j=1 ; j< dim - 1 ; j++) {
		table[i][j] += temp[i][j];
	    }
	}
    }

    table_free(temp);
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
    compute_func_t func = naive_sync;
    
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
    return 0;
}
