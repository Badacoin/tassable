#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

int dim = 512;
int **table;
bool **scheduled;

typedef struct {
    int line;
    int column;
} square;

static square square_create (int i, int j)
{
    square temp;
    temp.line = i;
    temp.column = j;
    return temp;
}

square *stack;
int height = 0;

int
get (int i, int j)
{
    return table[i][j];
}

void stack_init ()
{
    for (int i = 1 ; i < dim - 1 ; i++) {
	for (int j = 1 ; j < dim - 1 ; j++) {
	    if (table[i][j] < 4)
		scheduled[i][j] = false;
	    else {
		scheduled[i][j] = true;
	        stack[height] = square_create(i,j);
		height++;
	    }
	}
    }
}

bool task_seq (int iterations)
{
    while (iterations > 0 && height > 0) {
	height--;
	iterations--;
	square temp = stack[height];
	int i = temp.line;
	int j = temp.column;
	scheduled[i][j] = false;
	
	int mod4 = table[i][j] % 4;      
	int div4 = table[i][j] / 4;
	table[i][j] = mod4;
	table[i-1][j] += div4;   
	table[i+1][j] += div4;   
	table[i][j-1] += div4;   
	table[i][j+1] += div4;
	if ((i > 1) && (table[i-1][j] >= 4)  && !scheduled[i-1][j]) {
	    scheduled[i-1][j] = true;
	    stack[height] = square_create(i-1,j);
	    height++;
	}
	if ((i < dim - 2) && (table[i+1][j] >= 4) && !scheduled[i+1][j]) {
	    scheduled[i+1][j] = true;
	    stack[height] = square_create(i+1,j);
	    height++;
	}
	if ((j > 1) && (table[i][j-1] >= 4) && !scheduled[i][j-1]) {
	    scheduled[i][j-1] = true;
	    stack[height] = square_create(i,j-1);
	    height++;
	}
	if ((j < dim - 2) && (table[i][j+1] >= 4) && !scheduled[i][j+1]) {
	    scheduled[i][j+1] = true;
	    stack[height] = square_create(i,j+1);
	    height++;
	}
    }

    return (height == 0);
}

int
main (int argc, char **argv)
{
    bool graphical = false;
    bool check = false;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = task_seq;
    
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
    stack = malloc(dim * dim * sizeof(square));
    if (stack == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }

    bool *linear_scheduled = malloc(dim * dim * sizeof(bool));
    if (linear_scheduled == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    scheduled = malloc(dim * sizeof(bool *));
    if (scheduled == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    int offset = 0;
    for (int i = 0 ; i < dim ; i++) {
        scheduled[i] = &linear_scheduled[offset];
	offset += dim;
    }

    
    if (tower_height != 0) {
	tower_init(table, tower_height, dim);
    }
    else {
	flat_init(table, FLAT_HEIGHT, dim);
    }

    stack_init();
    
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
    free(stack);
    free(scheduled[0]);
    free(scheduled);
    return 0;
}
