PROG	:= naive naive_sync absorb task_seq absorb_openmp absorb_gpu
CFLAGS	:= -g -O3 -std=c99 -Wall -Wextra
LDLIBS	:= -lm -fopenmp -lOpenCL -lGL -lGLU -lglut

all: $(PROG)

%: %.c display.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

display.o: display.c display.h utils.h

utils.o: utils.c utils.h

clean:
	rm -rf *.o $(PROG)

.PHONY: default clean
