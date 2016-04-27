
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define DIM 512
#define MAX_HEIGHT  10

int table[DIM][DIM];
bool finished = false;

// vecteur de pixel renvoyé par compute  
struct {
  float R, G, B;
} couleurs[DIM][DIM];

// callback
unsigned get (unsigned x, unsigned y)
{
  return table[y][x];
}

// Tas de sable initial
static void sable_init (unsigned height)
{
  #pragma omp parallel for
  for (int y = 1; y < DIM - 1 ; y++)
    for (int x = 1; x < DIM -1 ; x++) {
      table[y][x] = height;
    }
}

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

static void tower_init (int height)
{
    for (int y = 0; y < DIM; y++) {
	for (int x = 0; x < DIM; x++) {
	    table[y][x] = 0;
	}
    }
    table[DIM/2][DIM/2] = height;
}

// callback
float *compute (unsigned iterations)
{
  static int step = 0;
  for (unsigned i = 0; i < iterations; i++)
    {
      step++;
      for (int y = 0; y < DIM; y++)
	{
	  int v =  MAX_HEIGHT * (1+sin( 4* (y+step) * 3.14/ DIM)) / 4;
	  for (int x = 0; x < DIM; x++)
	    table[y][x]  = v;
	}
    }
  return DYNAMIC_COLORING; // altitude-based coloring
  // return couleurs;
}

float *eboule (unsigned iterations)
{
    for (int k = 0 ; k < iterations ; k++) {
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
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
    return DYNAMIC_COLORING;
}

float *sync_eboule (unsigned iterations)
{
    int temp[DIM][DIM];

    for (int k = 0 ; k < iterations ; k++) {
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j = 1 ; j< DIM - 1 ; j++) {
		temp[i][j] = 0;
	    }
	}
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
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
	
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j=1 ; j< DIM - 1 ; j++) {
		table[i][j] += temp[i][j];
	    }
	}
    }
    
    return DYNAMIC_COLORING;
}


float *sync_eboule_openmp (unsigned iterations)
{
  
    int temp[DIM][DIM];
    #pragma omp parallel for
    for (int i = 1 ; i < DIM - 1 ; i++) {	
	for (int j = 1 ; j< DIM - 1 ; j++) {
	    temp[i][j] = 0;
	}
    }

    finished = true;

    for (unsigned k ; k < iterations; k++) {
	#pragma omp parallel
	{
	    #pragma omp for collapse(2)
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		
		    if (table[i][j] >= 4) {
			finished = false;
			int mod4 = table[i][j] % 4;      
			int div4 = table[i][j] / 4;
			int delta = mod4 - table[i][j];
		        #pragma omp atomic
			temp[i][j] += delta;
       		        #pragma omp atomic		
			temp[i-1][j] += div4;
	    	        #pragma omp atomic		
			temp[i+1][j] += div4;
		        #pragma omp atomic			
			temp[i][j-1] += div4;
		        #pragma omp atomic
			temp[i][j+1] += div4;   
		    }
		}
	    }
	    
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    table[i][j] += temp[i][j];
		    temp[i][j] = 0;
		}
	    }
	}
    }
    return DYNAMIC_COLORING;
}

float *sync_eboule_openmp2 (unsigned iterations)
{
    finished = true;
    int temp[DIM][DIM];
    int init[DIM][DIM];

    #pragma omp parallel for
    for (int i = 0 ; i < DIM - 1 ; i++) {
	for (int j = 0 ; j < DIM - 1 ; j++) {
	    init[i][j] = table[i][j];
	}
    }
    
    for (unsigned k ; k < iterations; k++) {
	#pragma omp parallel
	{
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    int middle = table[i][j] % 4;
		    int up = table[i-1][j] / 4;
		    int down = table[i+1][j] /4;
		    int left = table[i][j-1] / 4;
		    int right = table[i][j+1] / 4;
		    temp[i][j] = middle + up + down + left + right;
		}
	    }
	    
	    #pragma omp for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    table[i][j] = temp[i][j];
		}
	    }
	}
    }

    #pragma omp parallel for
    for (int i = 1 ; i < DIM - 1 ; i++) {	
	for (int j = 1 ; j < DIM - 1 ; j++) {
	    if (table[i][j] != init[i][j]) {
		finished = false;
	    }
	}
    }
    
    return DYNAMIC_COLORING;
}

float *sync_eboule_pong_openmp (unsigned iterations)
{
    finished = true;
    int init[DIM][DIM];
    int temp[DIM][DIM];

    for (int i = 0 ; i < DIM - 1 ; i++) {
	temp[i][0] = temp[0][i] = temp[i][DIM - 1] = temp[DIM - 1][i] = 0;
    }

    #pragma omp parallel for
    for (int i = 0 ; i < DIM - 1 ; i++) {
	for (int j = 0 ; j < DIM - 1 ; j++) {
	    init[i][j] = table[i][j];
	}
    }
    
    for (unsigned k ; k < iterations; k++) {

	if (k % 2 == 0) {
	    #pragma omp parallel for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    int middle = table[i][j] % 4;
		    int up = table[i-1][j] / 4;
		    int down = table[i+1][j] /4;
		    int left = table[i][j-1] / 4;
		    int right = table[i][j+1] / 4;
		    temp[i][j] = middle + up + down + left + right;
		}
	    }
	}

	else {
	    #pragma omp parallel for
	    for (int i = 1 ; i < DIM - 1 ; i++) {	
		for (int j = 1 ; j < DIM - 1 ; j++) {
		    int middle = temp[i][j] % 4;
		    int up = temp[i-1][j] / 4;
		    int down = temp[i+1][j] /4;
		    int left = temp[i][j-1] / 4;
		    int right = temp[i][j+1] / 4;
		    table[i][j] = middle + up + down + left + right;
		}
	    }
	}
        
    }

    if (iterations % 2 == 1) {
	#pragma omp parallel for
	for (int i = 1 ; i < DIM - 1 ; i++) {	
	    for (int j = 1 ; j < DIM - 1 ; j++) {
		table[i][j] = temp[i][j];
	    }
	}
    }

    #pragma omp parallel for
    for (int i = 1 ; i < DIM - 1 ; i++) {	
	for (int j = 1 ; j < DIM - 1 ; j++) {
	    if (table[i][j] != init[i][j]) {
		finished = false;
	    }
	}
    }
    
    return DYNAMIC_COLORING;
}

void task_eboule ()
{
    int counter = 0;
    square stack[DIM * DIM];
    int height = 0;
    bool scheduled[DIM][DIM];

    for (int i = 1 ; i < DIM-1 ; i++)
	for (int j = 1 ; j < DIM-1 ; j++) {
	    if (table[i][j] < 4)
		scheduled[i][j] = false;
	    else {
		scheduled[i][j] = true;
	        stack[height] = square_create(i,j);
		height++;
	    }
	}

    while (height > 0) {
	height--;
	counter++;
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
	if ((i > 1) && (table[i-1][j] >= 4) && !scheduled[i-1][j]) {
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

    printf("%d eboulements\n", counter);
}

void run (compute_func_t compute_func, unsigned iterations)
{
    int computeTime = 0;
    unsigned call_counter = 0;
    while (!finished) {
	finished = true;
	struct timeval t1,t2;
	gettimeofday (&t1,NULL);
	compute_func(iterations);
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

void run_once (once_func_t once_func)
{
    int computeTime = 0;
    unsigned call_counter = 0;
    struct timeval t1,t2;
    gettimeofday (&t1,NULL);
    once_func();
    gettimeofday (&t2,NULL);
    computeTime = TIME_DIFF(t1,t2);

    printf("total time : %.3f s\n", computeTime / (float) (1000 * 1000));
}

int main (int argc, char **argv)
{
    sable_init(5);

    run(sync_eboule_openmp2, 1);

    if (0) {
	display_init (argc, argv,
		      DIM,              // dimension ( = x = y) du tas
		      MAX_HEIGHT,       // hauteur maximale du tas
		      get,              // callback func
		      sync_eboule_pong_openmp);         // callback func
    }
    
    return 0;
}
