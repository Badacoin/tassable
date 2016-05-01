DIM?=128

PROG	:= naive naive_sync absorb task_seq runtime_absorb_openmp \
	absorb_openmp gpu numa_runtime_absorb_openmp \
	naive_openmp outward_naive_sync gpu_overlap
CFLAGS	:= -g -O3 -std=c99 -Wall -Wextra -DDIM=$(DIM) -fopenmp
LDLIBS	:= -lm -fopenmp -lOpenCL -lGL -lGLU -lglut

all: $(PROG)

%: %.c display.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

display.o: display.c display.h utils.h

utils.o: utils.c utils.h

clean:
	rm -rf *.o $(PROG)

.PHONY: default clean
