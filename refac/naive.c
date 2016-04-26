#include "display.h"
#include "utils.h"

#include <stddef.h>

bool
naive (int **table, int dim, int iterations)
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
    int dim = 128;
    int **table = table_alloc(dim);
    flat_init(table, 5, dim);
    
    //run(naive, table, dim, 10);

    if (1) {
	display_init (0, NULL, dim, table, get, naive);
    }
    
    return 0;
}
