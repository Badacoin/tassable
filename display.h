
#ifndef _DISPLAY_H_IS_DEF_
#define _DISPLAY_H_IS_DEF_

#include <sys/time.h>

typedef unsigned (*get_func_t) (unsigned x, unsigned y);
typedef float * (*compute_func_t) (unsigned iterations);

#define STATIC_COLORING    ((float *)NULL)
#define DYNAMIC_COLORING   ((float *)1)

#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

void display_init (int argc, char **argv,
		   unsigned dim, unsigned max_height,
		   get_func_t get_func,
		   compute_func_t compute_func);

#endif
