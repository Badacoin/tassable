#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int **table;

int
get (int i, int j)
{
    return table[i][j];
}

bool
naive_openmp (int iterations)
{
    bool finished = true;
    int local_finished = true;
    
    for (int k = 0 ; k < iterations ; k++) {
        #pragma omp parallel firstprivate(local_finished)
	{
	    
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i += 3) {
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    if (table[i][j] >= 4) {
			local_finished = false;
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

	    #pragma omp for
	    for (int i = 2 ; i < DIM - 1 ; i += 3) {
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    if (table[i][j] >= 4) {
			local_finished = false;
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

	    #pragma omp for
	    for (int i = 3 ; i < DIM - 1 ; i += 3) {
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    if (table[i][j] >= 4) {
			local_finished = false;
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

	    #pragma omp atomic
	    finished &= local_finished;
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
    compute_func_t func = naive_openmp;
    
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
    return 0;
}
