CC = gcc
MV = mv

CFLAGS = -O2
LDLIBS = -lSDL -ldSFMT -lm

../nbody : nbody
	$(MV) nbody ../bin/

nbody : draw-sdl.o initial-condition-solar.o nbody.o physics-verlet-brute.o rng-dSFMT.o

.PHONY : clean
clean :
	$(RM) *~ ../bin/nbody *.o
