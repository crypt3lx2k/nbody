CC = gcc

MV = mv
LN = ln -sf

CFLAGS  = -Ofast -Wall -Wextra
LDLIBS  = -ldSFMT -lm

include draw-flags.mk
include physics-flags.mk

OBJS = align_malloc.o draw.o initial-condition.o nbody.o physics.o rng.o

../bin/nbody : nbody
	$(MV) nbody $@

physics-brute :
	$(LN) physics-verlet-brute.c physics.c
	$(LN) physics-verlet-brute.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-brute-openmp :
	$(LN) physics-verlet-brute-openmp.c physics.c
	$(LN) physics-verlet-brute-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-sse :
	$(LN) physics-verlet-brute-sse.c physics.c
	$(LN) physics-verlet-brute-sse.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-sse-openmp :
	$(LN) physics-verlet-brute-sse-openmp.c physics.c
	$(LN) physics-verlet-brute-sse-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-avx :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-avx.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

physics-avx-openmp :
	$(LN) physics-verlet-brute-avx.c physics.c
	$(LN) physics-verlet-brute-avx-openmp.mk physics-flags.mk
	$(MAKE) clean
	$(MAKE)

nbody : $(OBJS)

.PHONY : clean
clean :
	$(RM) ../bin/nbody *.o
