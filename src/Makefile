CC = gcc

MV = mv
LN = ln -s -f

CFLAGS = -O2 -Wall -Wextra
LDLIBS = -lSDL -ldSFMT -lm

OBJS = draw.o initial-condition.o physics.o rng.o
SRCS = $(OBJS:.o=.c)

default :
	touch $(SRCS)
	touch draw-sdl.c initial-condition-solar.c physics-verlet-brute.c rng-dSFMT.c
	$(MAKE) ../bin/nbody

openmp :
	touch $(SRCS)
	touch draw-sdl.c initial-condition-solar.c physics-verlet-brute-openmp.c rng-dSFMT.c
	$(MAKE) ../bin/nbody CFLAGS="$(CFLAGS) -fopenmp" LDFLAGS="$(LDFLAGS) -fopenmp"

../bin/nbody : nbody
	$(MV) nbody $@

nbody : nbody.o $(OBJS)

draw.c : draw-sdl.c
	$(LN) $? $@

initial-condition.c : initial-condition-solar.c
	$(LN) $? $@

physics.c : physics-verlet-brute.c physics-verlet-brute-openmp.c
	$(LN) $? $@

rng.c : rng-dSFMT.c
	$(LN) $? $@

.PHONY : clean
clean :
	$(RM) ../bin/nbody *.o $(SRCS)
