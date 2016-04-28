#include "display.h"
#include "utils.h"

#include <stddef.h>
#include <getopt.h>
#include <stdlib.h>

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
    for (int i = 1 ; i < DIM - 1 ; i++) {
	for (int j = 1 ; j < DIM - 1 ; j++) {
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
	if ((i < DIM - 2) && (table[i+1][j] >= 4) && !scheduled[i+1][j]) {
	    scheduled[i+1][j] = true;
	    stack[height] = square_create(i+1,j);
	    height++;
	}
	if ((j > 1) && (table[i][j-1] >= 4) && !scheduled[i][j-1]) {
	    scheduled[i][j-1] = true;
	    stack[height] = square_create(i,j-1);
	    height++;
	}
	if ((j < DIM - 2) && (table[i][j+1] >= 4) && !scheduled[i][j+1]) {
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
    bool validation = false;
    int tower_height = 0;
    int iterations = 1;
    int optc;
    compute_func_t func = task_seq;
    
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
    stack = malloc(DIM * DIM * sizeof(square));
    if (stack == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }

    bool *linear_scheduled = malloc(DIM * DIM * sizeof(bool));
    if (linear_scheduled == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    scheduled = malloc(DIM * sizeof(bool *));
    if (scheduled == NULL) {
	fputs("Error: out of memory!\n", stderr);
	exit(EXIT_FAILURE);
    }
    int offset = 0;
    for (int i = 0 ; i < DIM ; i++) {
        scheduled[i] = &linear_scheduled[offset];
	offset += DIM;
    }

    
    if (tower_height != 0) {
	tower_init(table, tower_height, DIM);
    }
    else {
	flat_init(table, FLAT_HEIGHT, DIM);
    }

    stack_init();
    
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
    free(stack);
    free(scheduled[0]);
    free(scheduled);
    return 0;
}
